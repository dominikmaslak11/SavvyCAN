#include "esp32_wifi_connection.h"
#include "can_structs.h"
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

// ═══════════════════════════════════════════════════════════════════════════
// ESP32 WiFi TCP Connection — CAN over IP
// Protocol: "T#ID#D0...D7#timestamp\n" (ESP32→SavvyCAN)
//           "t#ID#D0...D7\n"            (SavvyCAN→ESP32)
// ═══════════════════════════════════════════════════════════════════════════

Esp32WifiConnection::Esp32WifiConnection(QString portName)
    : CANConnection(portName, "ESP32_WIFI", CANCon::ESP32_WIFI,
                    0, 500000, false, 0, 1, 1024, false)
    , mSocket(nullptr)
    , mPort(DEFAULT_PORT)
    , mConnecting(false)
{
    // Parse portName as "host:port" or "host"
    int colonPos = portName.lastIndexOf(':');
    if (colonPos > 0) {
        mHost = QHostAddress(portName.left(colonPos));
        bool ok;
        int p = portName.mid(colonPos + 1).toInt(&ok);
        if (ok && p > 0 && p < 65536) mPort = p;
    } else {
        mHost = QHostAddress(portName);
    }

    if (mHost.isNull()) {
        // Try hostname resolution (blocking in constructor — called from GUI thread)
        mHost = QHostAddress(QHostAddress::LocalHost);
        sendDebug("ESP32 WiFi: Invalid host, defaulting to localhost");
    }

    mReconnectTimer.setInterval(RECONNECT_MS);
    mReconnectTimer.setSingleShot(true);
    connect(&mReconnectTimer, &QTimer::timeout, this, &Esp32WifiConnection::connectToDevice);
}

Esp32WifiConnection::~Esp32WifiConnection()
{
    piStop();
}

void Esp32WifiConnection::piStarted()
{
    setStatus(CANCon::CONNECTED);
    connectToDevice();
}

void Esp32WifiConnection::piStop()
{
    mReconnectTimer.stop();
    if (mSocket) {
        mSocket->disconnect();
        mSocket->close();
        delete mSocket;
        mSocket = nullptr;
    }
    setStatus(CANCon::NOT_CONNECTED);
}

void Esp32WifiConnection::connectToDevice()
{
    if (mConnecting || (mSocket && mSocket->state() == QAbstractSocket::ConnectedState)) return;

    if (!mSocket) {
        mSocket = new QTcpSocket(this);
        connect(mSocket, &QTcpSocket::connected, this, &Esp32WifiConnection::deviceConnected);
        connect(mSocket, &QTcpSocket::disconnected, this, &Esp32WifiConnection::deviceDisconnected);
        connect(mSocket, &QTcpSocket::readyRead, this, &Esp32WifiConnection::readTcpData);
        connect(mSocket, &QTcpSocket::errorOccurred, this, &Esp32WifiConnection::onError);
    }

    mConnecting = true;
    sendDebug(QString("ESP32 WiFi: Connecting to %1:%2").arg(mHost.toString()).arg(mPort));
    mSocket->connectToHost(mHost, mPort);
}

void Esp32WifiConnection::deviceConnected()
{
    mConnecting = false;
    sendDebug(QString("ESP32 WiFi: Connected to %1:%2").arg(mHost.toString()).arg(mPort));
    setStatus(CANCon::CONNECTED);
}

void Esp32WifiConnection::deviceDisconnected()
{
    sendDebug("ESP32 WiFi: Disconnected, will reconnect...");
    setStatus(CANCon::NOT_CONNECTED);
    mReconnectTimer.start();
}

void Esp32WifiConnection::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    mConnecting = false;
    sendDebug(QString("ESP32 WiFi: Error — %1. Reconnecting...").arg(mSocket->errorString()));
    mReconnectTimer.start();
}

void Esp32WifiConnection::readTcpData()
{
    if (!mSocket) return;

    mBuffer += QString::fromLatin1(mSocket->readAll());

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

void Esp32WifiConnection::processLine(const QString &line)
{
    // Protocol: T#ID#D0D1D2D3D4D5D6D7#timestamp
    //           T#ID#D0D1D2D3D4D5D6D7           (timestamp optional)

    if (!line.startsWith('T') || line.length() < 4) return;

    int hash1 = line.indexOf('#', 1);
    int hash2 = line.indexOf('#', hash1 + 1);
    if (hash1 < 0 || hash2 < 0) return;

    bool ok;
    uint32_t id = line.mid(1, hash1 - 1).toUInt(&ok, 16);
    if (!ok) return;

    CANFrame frame;
    frame.setFrameId(id);
    frame.setExtendedFrameFormat(id > 0x7FF);

    QString dataStr = line.mid(hash1 + 1, hash2 - hash1 - 1);
    QByteArray payload;
    for (int i = 0; i + 1 < dataStr.length(); i += 2) {
        payload.append(static_cast<char>(dataStr.mid(i, 2).toInt(&ok, 16)));
        if (!ok) break;
    }
    frame.setPayload(payload);

    // Optional timestamp after second '#'
    int hash3 = line.indexOf('#', hash2 + 1);
    if (hash3 > 0) {
        quint64 ts = line.mid(hash2 + 1, hash3 - hash2 - 1).toULongLong(&ok);
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

bool Esp32WifiConnection::piSendFrame(const CANFrame &frame)
{
    if (!mSocket || mSocket->state() != QAbstractSocket::ConnectedState) return false;

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

void Esp32WifiConnection::sendBytesToDevice(const QByteArray &bytes)
{
    if (mSocket && mSocket->state() == QAbstractSocket::ConnectedState) {
        mSocket->write(bytes);
    }
}

void Esp32WifiConnection::piSetBusSettings(int pBusIdx, CANBus pBus)
{
    Q_UNUSED(pBusIdx)
    Q_UNUSED(pBus)
}

bool Esp32WifiConnection::piGetBusSettings(int pBusIdx, CANBus &pBus)
{
    Q_UNUSED(pBusIdx)
    Q_UNUSED(pBus)
    return true;
}

void Esp32WifiConnection::piSuspend(bool pSuspend)
{
    Q_UNUSED(pSuspend)
}

void Esp32WifiConnection::sendDebug(const QString &text)
{
    if (mConsoleOutput) {
        emit debugOutput(text);
    }
}
