#include "BluetoothManager.h"
#include "framestore.h"
#include <QDebug>

BluetoothManager::BluetoothManager(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    mDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(mDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::onDeviceDiscovered);
    connect(mDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::scanFinished);

    mSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(mSocket, &QBluetoothSocket::connected, this, &BluetoothManager::onSocketConnected);
    connect(mSocket, &QBluetoothSocket::disconnected, this, &BluetoothManager::onSocketDisconnected);
    connect(mSocket, &QBluetoothSocket::readyRead, this, &BluetoothManager::onSocketReadyRead);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    connect(mSocket, &QBluetoothSocket::errorOccurred, this, &BluetoothManager::onSocketError);
#else
    connect(mSocket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this, &BluetoothManager::onSocketError);
#endif
}

BluetoothManager::~BluetoothManager()
{
    disconnectDevice();
    stopScan();
}

void BluetoothManager::startScan()
{
    mDiscoveryAgent->start();
    qInfo() << "Bluetooth scan started...";
}

void BluetoothManager::stopScan()
{
    mDiscoveryAgent->stop();
}

void BluetoothManager::connectToDevice(const QString &address)
{
    if (mSocket->state() == QBluetoothSocket::ConnectedState)
        disconnectDevice();

    QBluetoothAddress addr(address);
    mSocket->connectToService(addr, QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
    qInfo() << "Connecting to" << address;
}

void BluetoothManager::disconnectDevice()
{
    if (mSocket->state() != QBluetoothSocket::UnconnectedState) {
        mSocket->close();
    }
}

bool BluetoothManager::isConnected() const
{
    return mConnected;
}

QVector<QBluetoothDeviceInfo> BluetoothManager::discoveredDevices() const
{
    return mDiscoveryAgent->discoveredDevices();
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    QString name = info.name();
    if (name.isEmpty()) name = info.address().toString();

    emit deviceDiscovered(name, info.address().toString());

    // Auto-detect adapter type from device name
    if (name.contains("OBD", Qt::CaseInsensitive) ||
        name.contains("ELM", Qt::CaseInsensitive) ||
        name.contains("Vgate", Qt::CaseInsensitive))
    {
        mElm327Mode = true;
        qInfo() << "ELM327 adapter detected:" << name;
    }
    else if (name.contains("ESP32", Qt::CaseInsensitive) ||
             name.contains("CAN", Qt::CaseInsensitive))
    {
        mElm327Mode = false;
        qInfo() << "ESP32 CAN adapter detected:" << name;
    }
}

void BluetoothManager::onSocketConnected()
{
    mConnected = true;
    qInfo() << "Bluetooth connected";

    if (mElm327Mode) {
        // Initialize ELM327
        mSocket->write("ATZ\r\n");     // Reset
        mSocket->write("ATE0\r\n");    // Echo off
        mSocket->write("ATL0\r\n");    // Linefeed off
        mSocket->write("ATH1\r\n");    // Headers on
        mSocket->write("ATSP6\r\n");   // CAN 11-bit 500k
    }

    emit connected();
}

void BluetoothManager::onSocketDisconnected()
{
    mConnected = false;
    emit disconnected();
}

void BluetoothManager::onSocketReadyRead()
{
    mBuffer.append(mSocket->readAll());

    if (mElm327Mode) {
        // ELM327 sends lines ending in \r
        while (mBuffer.contains('\r')) {
            int idx = mBuffer.indexOf('\r');
            QByteArray line = mBuffer.left(idx).trimmed();
            mBuffer.remove(0, idx + 1);
            parseElm327Line(line);
        }
    } else {
        // ESP32 sends binary CAN frames (14 bytes per frame)
        while (mBuffer.size() >= 14) {
            QByteArray frame = mBuffer.left(14);
            mBuffer.remove(0, 14);
            parseEsp32Frame(frame);
        }
    }
}

void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(mSocket->errorString());
}

void BluetoothManager::parseElm327Line(const QByteArray &line)
{
    // ELM327 format: "18DAF110 8 02 01 0D"
    // ID (8 hex) DLC data...
    if (line.length() < 10 || line.contains(">")) return;
    if (line.contains("SEARCHING") || line.contains("NO DATA")) return;
    if (line.startsWith("AT") || line.startsWith("OK")) return;

    QList<QByteArray> parts = line.split(' ');
    if (parts.size() < 3) return;

    bool ok;
    uint32_t id = parts[0].toUInt(&ok, 16);
    if (!ok) return;

    int dlc = parts[1].toInt();
    QByteArray data;
    for (int i = 0; i < dlc && (i + 2) < parts.size(); ++i)
        data.append(static_cast<char>(parts[i + 2].toUInt(nullptr, 16)));

    bool extended = (id > 0x7FF);

    CANFrame frame;
    frame.setFrameId(id);
    if (extended) frame.setExtendedFrameFormat(true);
    frame.isReceived = true;
    frame.setFrameType(QCanBusFrame::DataFrame);
    frame.bus = 0;
    frame.setPayload(data);
    frame.setTimeStamp(QCanBusFrame::TimeStamp(0, QDateTime::currentMSecsSinceEpoch() * 1000LL));

    mStore->addFrame(frame);
    emit frameReceived(id, data, extended);
}

void BluetoothManager::parseEsp32Frame(const QByteArray &frame)
{
    // ESP32 binary protocol: [marker 2B][id 4B][dlc 1B][data 8B][crc 1B]
    if (frame.size() < 14) return;
    if (static_cast<uint8_t>(frame[0]) != 0xAA || static_cast<uint8_t>(frame[1]) != 0x55) return;

    uint32_t id = (static_cast<uint8_t>(frame[2]) << 24) |
                  (static_cast<uint8_t>(frame[3]) << 16) |
                  (static_cast<uint8_t>(frame[4]) << 8)  |
                   static_cast<uint8_t>(frame[5]);

    int dlc = static_cast<uint8_t>(frame[6]) & 0x0F;
    bool extended = (static_cast<uint8_t>(frame[6]) & 0x80) != 0;

    QByteArray data = frame.mid(7, qMin(dlc, 8));

    CANFrame canFrame;
    canFrame.setFrameId(id);
    if (extended) canFrame.setExtendedFrameFormat(true);
    canFrame.isReceived = true;
    canFrame.setFrameType(QCanBusFrame::DataFrame);
    canFrame.bus = 0;
    canFrame.setPayload(data);
    canFrame.setTimeStamp(QCanBusFrame::TimeStamp(0, QDateTime::currentMSecsSinceEpoch() * 1000LL));

    mStore->addFrame(canFrame);
    emit frameReceived(id, data, extended);
}

void BluetoothManager::sendFrame(uint32_t id, const QByteArray &data, bool extended)
{
    if (!mConnected || !mSocket) return;

    if (mElm327Mode) {
        // ELM327 AT command for sending
        QString cmd = QString("ATSH%1\r\n").arg(id, extended ? 8 : 3, 16, QChar('0'));
        mSocket->write(cmd.toUtf8());

        QByteArray frame;
        QTextStream stream(&frame);
        stream << (extended ? "ATCE" : "ATCS") << "\r\n";
        stream << QString::number(data.size()) << " ";
        for (int i = 0; i < data.size(); ++i)
            stream << QString("%1 ").arg(static_cast<uint8_t>(data[i]), 2, 16, QChar('0'));
        stream << "\r\n";
        mSocket->write(frame);
    } else {
        // ESP32 binary protocol
        QByteArray frame;
        frame.append('\xAA');
        frame.append('\x55');
        frame.append(static_cast<char>((id >> 24) & 0xFF));
        frame.append(static_cast<char>((id >> 16) & 0xFF));
        frame.append(static_cast<char>((id >> 8) & 0xFF));
        frame.append(static_cast<char>(id & 0xFF));
        uint8_t flags = (extended ? 0x80 : 0x00) | (data.size() & 0x0F);
        frame.append(static_cast<char>(flags));
        frame.append(data.left(8));
        while (frame.size() < 14)
            frame.append('\x00');
        // Simple XOR checksum
        uint8_t crc = 0;
        for (int i = 0; i < 13; ++i)
            crc ^= static_cast<uint8_t>(frame[i]);
        frame.append(static_cast<char>(crc));
        mSocket->write(frame);
    }
}
