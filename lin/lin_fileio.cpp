#include "lin_fileio.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

LINFileIO::LINFileIO() {}

// ═══════════════════════════════════════════════════════════════════════════
// LDF Parser
// ═══════════════════════════════════════════════════════════════════════════

bool LINFileIO::loadLDF(QString filename, LDFDatabase &db)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        db.errorString = QString("Cannot open file: %1").arg(filename);
        return false;
    }

    QTextStream in(&file);
    db = LDFDatabase(); // reset

    enum Section { None, ProtocolVersion, LanguageVersion, Speed, Channel,
                   Nodes, Signals, Frames, ScheduleEntries };
    Section currentSection = None;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("//") || line.startsWith("--"))
            continue;

        // Section headers
        if (line.contains("LIN_description_file", Qt::CaseInsensitive)) {
            db.protocolVersion = "2.2";
            currentSection = None;
        } else if (line.contains("LIN_language_version")) {
            currentSection = LanguageVersion;
        } else if (line.contains("LIN_protocol_version")) {
            currentSection = ProtocolVersion;
        } else if (line.contains("LIN_speed")) {
            currentSection = Speed;
        } else if (line.contains("Channel_name")) {
            currentSection = Channel;
        } else if (line.contains("Nodes", Qt::CaseInsensitive) && line.contains("{")) {
            currentSection = Nodes;
        } else if (line.contains("Signals", Qt::CaseInsensitive) && line.contains("{")) {
            currentSection = Signals;
        } else if (line.contains("Frames", Qt::CaseInsensitive) && line.contains("{")) {
            currentSection = Frames;
        } else if (line.contains("Schedule_tables", Qt::CaseInsensitive) && line.contains("{")) {
            currentSection = ScheduleEntries;
        }

        // Parse based on section
        switch (currentSection) {
            case ProtocolVersion: {
                int pos = 0;
                db.protocolVersion = readQuotedString(line, pos);
                break;
            }
            case LanguageVersion: {
                int pos = 0;
                db.languageVersion = readQuotedString(line, pos);
                break;
            }
            case Speed: {
                QRegularExpression re("(\\d+)\\s*kbps");
                auto match = re.match(line);
                if (match.hasMatch())
                    db.baudRate = match.captured(1).toUInt() * 1000;
                break;
            }
            case Channel: {
                int pos = 0;
                db.channelName = readQuotedString(line, pos);
                break;
            }
            case Nodes: {
                if (line.contains("Master") && line.contains("{")) {
                    LDFNode node;
                    node.isMaster = true;
                    // parse node attributes
                    while (!in.atEnd()) {
                        line = in.readLine().trimmed();
                        if (line.contains("}")) break;
                        int pos = 0;
                        QString key = readToken(line, pos);
                        if (key == "LIN_protocol")
                            node.protocolVer = readQuotedString(line, pos).toDouble();
                        else if (key == "configured_NAD")
                            node.NAD = (uint8_t)readInteger(line, pos);
                        else if (key == "supplier_id")
                            node.supplierId = (uint8_t)readInteger(line, pos);
                        else if (key == "function_id")
                            node.functionId = (uint8_t)readInteger(line, pos);
                        else if (key == "variant_id")
                            node.variantId = (uint8_t)readInteger(line, pos);
                        else if (pos == 0) // first token = name
                            node.name = key;
                    }
                    db.nodes.append(node);
                } else if (line.contains("Slave") && line.contains("{")) {
                    LDFNode node;
                    node.isMaster = false;
                    while (!in.atEnd()) {
                        line = in.readLine().trimmed();
                        if (line.contains("}")) break;
                        int pos = 0;
                        QString key = readToken(line, pos);
                        if (key == "LIN_protocol")
                            node.protocolVer = readQuotedString(line, pos).toDouble();
                        else if (key == "configured_NAD")
                            node.NAD = (uint8_t)readInteger(line, pos);
                        else if (pos == 0)
                            node.name = key;
                    }
                    db.nodes.append(node);
                }
                break;
            }
            case Signals: {
                if (line.contains(":") && !line.contains("{")) {
                    LINSignal sig;
                    int pos = 0;
                    sig.name = readToken(line, pos); // e.g. "SignalName:"
                    sig.name.chop(1); // remove trailing colon
                    skipWhitespace(line, pos);

                    sig.length = (uint8_t)readInteger(line, pos);
                    skipWhitespace(line, pos);
                    sig.startBit = (uint8_t)readInteger(line, pos);

                    if (pos < line.length()) {
                        QString publisher = readToken(line, pos);
                        sig.publisher = publisher;
                        if (pos < line.length()) {
                            QString subscriber = readToken(line, pos);
                            sig.subscribers.append(subscriber);
                        }
                    }
                    db.signalList.append(sig);
                }
                break;
            }
            case Frames: {
                if (line.contains(":") && !line.contains("{")) {
                    uint8_t frameId = 0;
                    QString frameName;
                    int len = 0;
                    QString publisher;

                    int pos = 0;
                    QString first = readToken(line, pos);
                    if (first.endsWith(":")) {
                        // "FrameName:"
                        frameName = first;
                        frameName.chop(1);

                        // parse: ID, Length, Publisher
                        if (pos < line.length()) {
                            frameId = (uint8_t)readInteger(line, pos);
                        }
                        if (pos < line.length()) {
                            len = readInteger(line, pos);
                        }
                        if (pos < line.length()) {
                            publisher = readToken(line, pos);
                        }
                        db.frameNames[frameId] = frameName;

                        // Create schedule entry
                        LINScheduleEntry entry;
                        entry.frameId = frameId;
                        entry.frameName = frameName;
                        entry.delay_ms = 50; // default
                        db.schedule.append(entry);
                    }
                }
                break;
            }
            case ScheduleEntries: {
                // Parse schedule table entries
                if (line.contains("delay") || line.contains("Delay")) {
                    // Just capture schedule entries here
                }
                break;
            }
            default:
                break;
        }
    }

    file.close();
    db.isValid = true;
    return true;
}

bool LINFileIO::saveLDF(QString filename, const LDFDatabase &db)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "LIN_description_file;" << Qt::endl;
    out << "LIN_protocol_version = \"" << db.protocolVersion << "\";" << Qt::endl;
    out << "LIN_language_version = \"" << db.languageVersion << "\";" << Qt::endl;
    out << "LIN_speed = " << (db.baudRate / 1000.0) << " kbps;" << Qt::endl;
    out << Qt::endl;

    out << "Nodes {" << Qt::endl;
    for (const auto &node : db.nodes) {
        out << "    " << (node.isMaster ? "Master" : "Slave") << ": " << node.name << " {" << Qt::endl;
        out << "        LIN_protocol = \"" << node.protocolVer << "\";" << Qt::endl;
        if (node.NAD)
            out << "        configured_NAD = " << node.NAD << ";" << Qt::endl;
        out << "    }" << Qt::endl;
    }
    out << "}" << Qt::endl << Qt::endl;

    out << "Signals {" << Qt::endl;
    for (const auto &sig : db.signalList) {
        out << "    " << sig.name << ": " << sig.length << ", " << sig.startBit
            << ", " << sig.publisher;
        for (const auto &sub : sig.subscribers)
            out << ", " << sub;
        out << ";" << Qt::endl;
    }
    out << "}" << Qt::endl << Qt::endl;

    out << "Frames {" << Qt::endl;
    for (auto it = db.frameNames.begin(); it != db.frameNames.end(); ++it) {
        // Determine length from signals
        int maxLen = 1;
        for (const auto &sig : db.signalList) {
            if (sig.frameId == it.key()) {
                int endBit = sig.startBit + sig.length;
                int endByte = (endBit + 7) / 8;
                if (endByte > maxLen) maxLen = endByte;
            }
        }
        out << "    " << it.value() << ": " << it.key() << ", " << maxLen
            << ";" << Qt::endl;
    }
    out << "}" << Qt::endl << Qt::endl;

    out << "Schedule_tables {" << Qt::endl;
    out << "    MainSchedule {" << Qt::endl;
    for (const auto &entry : db.schedule) {
        out << "        " << entry.frameName << " delay " << entry.delay_ms
            << " ms;" << Qt::endl;
    }
    out << "    }" << Qt::endl;
    out << "}" << Qt::endl;

    file.close();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// LIN Log Formats
// ═══════════════════════════════════════════════════════════════════════════

bool LINFileIO::loadLINLogGeneric(QString filename, QVector<LINFrame> &frames)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    frames.clear();
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#") || line.startsWith("//"))
            continue;

        // Format: timestamp,id,data(hex),checksum
        // e.g.: 123456,0x2A,FF A0 01,0x5C
        QStringList parts = line.split(',');
        if (parts.size() < 3)
            continue;

        LINFrame frame;
        frame.timestamp = parts[0].toULongLong();

        QString idStr = parts[1].trimmed();
        if (idStr.startsWith("0x", Qt::CaseInsensitive))
            frame.id = (uint8_t)idStr.mid(2).toUInt(nullptr, 16);
        else
            frame.id = (uint8_t)idStr.toUInt();

        frame.pid = LINHelpers::buildPID(frame.id);

        // Parse data bytes in hex
        QStringList dataBytes = parts[2].split(' ', Qt::SkipEmptyParts);
        frame.dataLen = qMin(dataBytes.size(), LIN::MAX_DATA_LEN);
        for (int i = 0; i < frame.dataLen; ++i) {
            bool ok;
            frame.data[i] = (uint8_t)dataBytes[i].toUInt(&ok, 16);
        }

        if (parts.size() >= 4) {
            QString csStr = parts[3].trimmed();
            if (csStr.startsWith("0x", Qt::CaseInsensitive))
                frame.checksum = (uint8_t)csStr.mid(2).toUInt(nullptr, 16);
            else
                frame.checksum = (uint8_t)csStr.toUInt();
        }

        // Calculate checksum
        frame.checksumType = LINChecksumType::Enhanced;
        frame.calcChecksum = LINHelpers::calcEnhancedChecksum(frame.pid, frame.data, frame.dataLen);

        frames.append(frame);
    }

    file.close();
    return true;
}

bool LINFileIO::saveLINLogCSV(QString filename, const QVector<LINFrame> &frames)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "# LIN Log generated by SavvyCAN" << Qt::endl;
    out << "# Timestamp,ID,Data,Checksum" << Qt::endl;

    for (const auto &frame : frames) {
        out << frame.timestamp << ","
            << "0x" << QString::number(frame.id, 16).toUpper() << ","
            << frame.dataHex() << ","
            << "0x" << QString::number(frame.checksum, 16).toUpper()
            << Qt::endl;
    }

    file.close();
    return true;
}

bool LINFileIO::saveLINLogNative(QString filename, const QVector<LINFrame> &frames)
{
    // Native binary format for efficiency
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // Header
    out.writeRawData("LINF", 4);
    out << (uint16_t)1; // version
    out << (uint32_t)frames.size();

    for (const auto &frame : frames) {
        out << (quint64)frame.timestamp;
        out << frame.pid;
        out << frame.dataLen;
        out.writeRawData((const char*)frame.data, frame.dataLen);
        out << frame.checksum;
        out << (uint8_t)frame.errorFlags;
    }

    file.close();
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Detection
// ═══════════════════════════════════════════════════════════════════════════

bool LINFileIO::isLDFFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    QString line = in.readLine();
    file.close();

    return line.contains("LIN_description_file", Qt::CaseInsensitive);
}

bool LINFileIO::isLINLogFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    // Check first few lines for LIN frame patterns
    for (int i = 0; i < 10 && !in.atEnd(); ++i) {
        QString line = in.readLine().trimmed();
        if (line.contains("LIN") || line.contains("lin"))
            return true;
        // Check if it looks like a LIN frame CSV
        if (line.contains(',') && line.contains("0x")) {
            QStringList parts = line.split(',');
            if (parts.size() >= 3) {
                bool ok;
                uint32_t id = parts[1].trimmed().mid(2).toUInt(&ok, 16);
                if (ok && id <= 63) // LIN IDs are 0-63
                    return true;
            }
        }
    }
    file.close();
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// Helper functions for LDF parsing
// ═══════════════════════════════════════════════════════════════════════════

void LINFileIO::skipWhitespace(const QString &line, int &pos)
{
    while (pos < line.length() && line[pos].isSpace())
        ++pos;
}

QString LINFileIO::readToken(const QString &line, int &pos)
{
    skipWhitespace(line, pos);
    if (pos >= line.length())
        return {};

    if (line[pos] == '"')
        return readQuotedString(line, pos);

    int start = pos;
    while (pos < line.length() && !line[pos].isSpace() &&
           line[pos] != ',' && line[pos] != ';' && line[pos] != ':' &&
           line[pos] != '{' && line[pos] != '}')
        ++pos;

    QString tok = line.mid(start, pos - start);
    skipWhitespace(line, pos);
    if (pos < line.length() && (line[pos] == ',' || line[pos] == ';')) {
        ++pos; // skip delimiter
    }
    return tok;
}

QString LINFileIO::readQuotedString(const QString &line, int &pos)
{
    skipWhitespace(line, pos);
    if (pos >= line.length() || line[pos] != '"')
        return {};

    ++pos; // skip opening quote
    int start = pos;
    while (pos < line.length() && line[pos] != '"')
        ++pos;

    QString val = line.mid(start, pos - start);
    if (pos < line.length())
        ++pos; // skip closing quote
    skipWhitespace(line, pos);
    if (pos < line.length() && (line[pos] == ',' || line[pos] == ';'))
        ++pos;
    return val;
}

int LINFileIO::readInteger(const QString &line, int &pos)
{
    skipWhitespace(line, pos);
    if (pos >= line.length())
        return 0;

    int start = pos;
    while (pos < line.length() && (line[pos].isDigit() || line[pos] == '-'))
        ++pos;

    bool ok;
    int val = line.mid(start, pos - start).toInt(&ok);

    skipWhitespace(line, pos);
    if (pos < line.length() && (line[pos] == ',' || line[pos] == ';'))
        ++pos;

    return ok ? val : 0;
}

double LINFileIO::readDouble(const QString &line, int &pos)
{
    skipWhitespace(line, pos);
    if (pos >= line.length())
        return 0.0;

    int start = pos;
    while (pos < line.length() && (line[pos].isDigit() || line[pos] == '.' || line[pos] == '-'))
        ++pos;

    bool ok;
    double val = line.mid(start, pos - start).toDouble(&ok);

    skipWhitespace(line, pos);
    if (pos < line.length() && (line[pos] == ',' || line[pos] == ';'))
        ++pos;

    return ok ? val : 0.0;
}
