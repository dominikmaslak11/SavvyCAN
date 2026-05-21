#include "esp32_ble_connection.h"
#include "can_structs.h"
#include <QDebug>
#include <QDateTime>
#include <QStringList>

// ═══════════════════════════════════════════════════════════════════════════
// ESP32 Bluetooth SPP Connection — CAN over Bluetooth
// Protocol: "T#ID#D0...D7#timestamp\n" (ESP32→SavvyCAN)
//           "t#ID#D0...D7\n"            (SavvyCAN→ESP32)
// ═══════════════════════════════════════════════════════════════════════════

Esp32BleConnection::Esp32BleConnection(QString portName)
    : CANConnection(portName, "ESP32_BLE", CANCon::ESP32_BLE,
                    DEFAULT_BAUD, DEFAULT_CAN_SPEED, false, 0, 1, 1024, true)
    , mSerial(nullptr)
    , mPortName(portName)
    , mBaudRate(DEFAULT_BAUD)
    , mStopping(false)
{
}

Esp32BleConnection::~Esp32BleConnection()
{
    piStop();
}

void Esp32BleConnection::piStarted()
{
    mStopping = false;
    mBuffer.clear();

    mSerial = new QSerialPort(mPortName, this);
    mSerial->setBaudRate(mBaudRate);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setFlowControl(QSerialPort::NoFlowControl);

    connect(mSerial, &QSerialPort::readyRead, this, &Esp32BleConnection::readSerialData);
    connect(mSerial, &QSerialPort::errorOccurred, this, &Esp32BleConnection::onSerialError);

    if (mSerial->open(QIODevice::ReadWrite)) {
        sendDebug(QString("ESP32 BLE: Connected to %1 @ %2 baud")
                  .arg(mPortName).arg(mBaudRate));
        updateStatus(CANCon::CONNECTED);
    } else {
        sendDebug(QString("ESP32 BLE: Failed to open %1 — %2")
                  .arg(mPortName).arg(mSerial->errorString()));
        closeSerialPort();
        updateStatus(CANCon::NOT_CONNECTED);
    }
}

void Esp32BleConnection::piStop()
{
    mStopping = true;
    closeSerialPort();
    updateStatus(CANCon::NOT_CONNECTED);
}

void Esp32BleConnection::closeSerialPort()
{
    if (!mSerial) return;

    mSerial->disconnect(this);
    if (mSerial->isOpen()) {
        mSerial->clear();
        mSerial->close();
    }
    mSerial->deleteLater();
    mSerial = nullptr;
}

void Esp32BleConnection::readSerialData()
{
    if (!mSerial) return;

    mBuffer += QString::fromLatin1(mSerial->readAll());

    // Process complete lines
    int newlinePos;
    while ((newlinePos = mBuffer.indexOf('\n')) >= 0) {
        QString line = mBuffer.left(newlinePos).trimmed();
        mBuffer = mBuffer.mid(newlinePos + 1);
        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void Esp32BleConnection::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || mStopping || !mSerial) return;

    sendDebug(QString("ESP32 BLE: Serial error — %1").arg(mSerial->errorString()));
    closeSerialPort();
    updateStatus(CANCon::NOT_CONNECTED);
}

void Esp32BleConnection::processLine(const QString &line)
{
    // Protocol: T#ID#D0D1D2D3D4D5D6D7#timestamp
    //           T#ID#D0D1D2D3D4D5D6D7           (timestamp optional)

    if (!line.startsWith("T#")) return;

    bool ok;
    const QStringList parts = line.split('#');
    if (parts.size() < 3) return;

    uint32_t id = parts[1].toUInt(&ok, 16);
    if (!ok) return;

    CANFrame frame;
    const bool ext = (id & 0x80000000UL) != 0 || id > 0x7FF;
    id &= 0x1FFFFFFFUL;
    frame.setFrameId(id);
    frame.setExtendedFrameFormat(ext);

    QString dataStr = parts[2];
    if (dataStr.length() > 16) dataStr.truncate(16);
    QByteArray payload;
    for (int i = 0; i + 1 < dataStr.length(); i += 2) {
        const int byte = dataStr.mid(i, 2).toInt(&ok, 16);
        if (!ok) break;
        payload.append(static_cast<char>(byte));
    }
    frame.setPayload(payload);

    // Optional timestamp after second '#'
    if (parts.size() > 3) {
        quint64 ts = parts[3].toULongLong(&ok);
        if (ok) {
            frame.setTimeStamp(QCanBusFrame::TimeStamp(0, ts));
        }
    } else {
        frame.setTimeStamp(QCanBusFrame::TimeStamp(
            0, QDateTime::currentMSecsSinceEpoch() * 1000));
    }

    // Push to SavvyCAN's frame queue
    CANFrame *slot = getQueue().get();
    if (slot) {
        *slot = frame;
        getQueue().queue();
    }
    checkTargettedFrame(frame);
}

bool Esp32BleConnection::piSendFrame(const CANFrame &frame)
{
    if (!mSerial || !mSerial->isOpen()) return false;

    // Protocol: t#ID#D0D1D2D3D4D5D6D7\n
    QString cmd;
    cmd += 't';
    cmd += '#';
    cmd += QString::number(frame.frameId(), 16).rightJustified(8, '0').toUpper();
    cmd += '#';

    const QByteArray &data = frame.payload();
    for (int i = 0; i < data.length(); i++) {
        cmd += QString::number(static_cast<uint8_t>(data[i]), 16).rightJustified(2, '0').toUpper();
    }
    cmd += '\n';

    sendBytesToDevice(cmd.toLatin1());
    return true;
}

void Esp32BleConnection::sendBytesToDevice(const QByteArray &bytes)
{
    if (mSerial && mSerial->isOpen()) {
        mSerial->write(bytes);
    }
}

void Esp32BleConnection::piSetBusSettings(int pBusIdx, CANBus pBus)
{
    Q_UNUSED(pBusIdx)
    Q_UNUSED(pBus)
}

bool Esp32BleConnection::piGetBusSettings(int pBusIdx, CANBus &pBus)
{
    Q_UNUSED(pBusIdx)
    pBus.setSpeed(DEFAULT_CAN_SPEED);
    pBus.setActive(true);
    pBus.setListenOnly(false);
    pBus.setCanFD(false);
    return true;
}

void Esp32BleConnection::piSuspend(bool pSuspend)
{
    Q_UNUSED(pSuspend)
}

void Esp32BleConnection::sendDebug(const QString &text)
{
    qDebug() << text;
    if (mConsoleOutput) {
        emit debugOutput(text);
    }
}

void Esp32BleConnection::updateStatus(CANCon::status statusValue)
{
    setStatus(statusValue);

    CANConStatus stats;
    stats.conStatus = statusValue;
    stats.numHardwareBuses = getNumBuses();
    emit status(stats);
}
