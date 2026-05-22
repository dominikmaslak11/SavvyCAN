#include "WifiManager.h"
#include "can_structs.h"
#include <QDebug>
#include <QDateTime>

WifiManager::WifiManager(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    mSocket = new QTcpSocket(this);
    connect(mSocket, &QTcpSocket::connected, this, &WifiManager::onSocketConnected);
    connect(mSocket, &QTcpSocket::disconnected, this, &WifiManager::onSocketDisconnected);
    connect(mSocket, &QTcpSocket::readyRead, this, &WifiManager::onSocketReadyRead);
    connect(mSocket, &QTcpSocket::errorOccurred, this, &WifiManager::onSocketError);
}

WifiManager::~WifiManager()
{
    disconnectDevice();
}

void WifiManager::connectToDevice(const QString &ip, int port)
{
    disconnectDevice();
    mSocket->connectToHost(ip, port);
}

void WifiManager::disconnectDevice()
{
    if (mSocket->state() != QAbstractSocket::UnconnectedState) {
        mSocket->disconnectFromHost();
    }
}

bool WifiManager::isConnected() const
{
    return mSocket->state() == QAbstractSocket::ConnectedState;
}

void WifiManager::sendFrame(uint32_t id, const QByteArray &data, bool extended)
{
    if (!isConnected()) return;
    
    // GVRET / ESP32 WiFi simple format for transmit:
    // f1 00 id_l id_h id_u id_msb dlc data0 ... data7
    QByteArray pkt;
    pkt.append('\xF1'); // command: transmit
    pkt.append('\x00'); // CAN bus 0
    pkt.append(static_cast<char>(id & 0xFF));
    pkt.append(static_cast<char>((id >> 8) & 0xFF));
    pkt.append(static_cast<char>((id >> 16) & 0xFF));
    
    uint8_t msb = (id >> 24) & 0x1F;
    if (extended) msb |= 0x80;
    pkt.append(static_cast<char>(msb));
    
    pkt.append(static_cast<char>(data.size()));
    pkt.append(data);
    pkt.append('\x00'); // padding
    
    mSocket->write(pkt);
}

void WifiManager::onSocketConnected()
{
    emit connectionChanged(true);
}

void WifiManager::onSocketDisconnected()
{
    emit connectionChanged(false);
}

void WifiManager::onSocketReadyRead()
{
    mBuffer.append(mSocket->readAll());
    
    // Naive frame parser for ESP32/GVRET (F1 ... or simple line-based)
    // To be robust, this depends on exact protocol. Assuming standard SavvyCAN ESP32 over TCP.
    // If it's SLCAN over TCP:
    while (mBuffer.contains('\r')) {
        int idx = mBuffer.indexOf('\r');
        QByteArray line = mBuffer.left(idx).trimmed();
        mBuffer.remove(0, idx + 1);
        
        if (line.isEmpty()) continue;
        
        // SLCAN detection "t1238112233..."
        if (line.startsWith('t') || line.startsWith('T')) {
            parseGvretFrame(line);
        }
    }
}

void WifiManager::onSocketError(QTcpSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(mSocket->errorString());
}

void WifiManager::parseGvretFrame(const QByteArray &frame)
{
    if (!mStore) return;
    
    CANFrame f;
    f.bus = 0;
    f.setTimeStamp(QCanBusFrame::TimeStamp(0, QDateTime::currentMSecsSinceEpoch() * 1000));
    
    bool isExt = frame.startsWith('T');
    int idLen = isExt ? 8 : 3;
    if (frame.size() < 1 + idLen + 1) return;
    
    bool ok;
    f.setFrameId(frame.mid(1, idLen).toUInt(&ok, 16));
    f.setExtendedFrameFormat(isExt);
    
    int dlc = frame.mid(1 + idLen, 1).toUInt(&ok, 16);
    if (!ok || dlc < 0 || dlc > 64) return;
    
    if (frame.size() >= 1 + idLen + 1 + dlc * 2) {
        f.setPayload(QByteArray::fromHex(frame.mid(1 + idLen + 1, dlc * 2)));
    }
    
    mStore->addFrame(f);
}
