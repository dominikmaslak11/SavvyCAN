#ifndef ESP32_BLE_CONNECTION_H
#define ESP32_BLE_CONNECTION_H

#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "canconnection.h"
#include "canconmanager.h"

/**
 * @brief ESP32 over Bluetooth connection
 *
 * Connects to an ESP32 via Bluetooth Classic SPP (Serial Port Profile).
 * The ESP32 appears as a virtual COM port (Windows) or /dev/rfcomm* (Linux).
 *
 * Protocol (text-based, GVRET-compatible):
 *   ESP32 → SavvyCAN:  T#ID#D0D1D2D3D4D5D6D7#timestamp\n
 *   SavvyCAN → ESP32:  t#ID#D0D1D2D3D4D5D6D7\n
 *
 * The ESP32 must be running the companion firmware (see esp32_firmware/).
 */
class Esp32BleConnection : public CANConnection
{
    Q_OBJECT

public:
    explicit Esp32BleConnection(QString portName);
    ~Esp32BleConnection() override;

    CANCon::type getType() const noexcept override {
        return CANCon::ESP32_BLE;
    }

    int getSerialSpeed() const noexcept {
        return mBaudRate;
    }

protected:
    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

private slots:
    void readSerialData();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    void closeSerialPort();
    void processLine(const QString &line);
    void sendBytesToDevice(const QByteArray &bytes);
    void sendDebug(const QString &text);
    void updateStatus(CANCon::status status);

    QSerialPort  *mSerial;
    QString       mPortName;
    int           mBaudRate;
    QString       mBuffer;          // partial line buffer
    bool          mStopping;

    static constexpr int DEFAULT_BAUD = 115200;
    static constexpr int DEFAULT_CAN_SPEED = 500000;
};

#endif // ESP32_BLE_CONNECTION_H
