#ifndef ESP32_WIFI_CONNECTION_H
#define ESP32_WIFI_CONNECTION_H

#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>
#include "canconnection.h"
#include "canconmanager.h"

/**
 * @brief ESP32 over WiFi TCP connection
 *
 * Connects to an ESP32 running the CAN-over-TCP firmware.
 * Protocol (text-based, GVRET-compatible):
 *   ESP32 → SavvyCAN:  T#ID#D0D1D2D3D4D5D6D7#timestamp\n
 *   SavvyCAN → ESP32:  t#ID#D0D1D2D3D4D5D6D7\n
 *
 * The ESP32 must be running the companion firmware (see esp32_firmware/).
 */
class Esp32WifiConnection : public CANConnection
{
    Q_OBJECT

public:
    explicit Esp32WifiConnection(QString portName);
    ~Esp32WifiConnection() override;

    CANCon::type getType() const noexcept override {
        return CANCon::ESP32_WIFI;
    }

protected:
    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

private slots:
    void connectToDevice();
    void deviceConnected();
    void deviceDisconnected();
    void readTcpData();
    void onError(QAbstractSocket::SocketError error);

private:
    void processLine(const QString &line);
    void sendBytesToDevice(const QByteArray &bytes);
    void sendDebug(const QString &text);

    QTcpSocket   *mSocket;
    QHostAddress  mHost;
    int           mPort;
    QTimer        mReconnectTimer;
    QString       mBuffer;          // partial line buffer
    bool          mConnecting;

    static constexpr int DEFAULT_PORT = 35000;
    static constexpr int RECONNECT_MS = 3000;
};

#endif // ESP32_WIFI_CONNECTION_H
