#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include "framestore.h"

class WifiManager : public QObject
{
    Q_OBJECT
public:
    explicit WifiManager(FrameStore *store, QObject *parent = nullptr);
    ~WifiManager();

    Q_INVOKABLE void connectToDevice(const QString &ip, int port);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE bool isConnected() const;
    Q_INVOKABLE void sendFrame(uint32_t id, const QByteArray &data, bool extended);

signals:
    void connectionChanged(bool connected);
    void errorOccurred(const QString &errorMsg);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QTcpSocket::SocketError error);

private:
    void parseGvretFrame(const QByteArray &frame);

    QTcpSocket *mSocket;
    FrameStore *mStore;
    QByteArray mBuffer;
};

#endif // WIFIMANAGER_H
