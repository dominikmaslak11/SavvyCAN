#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QBluetoothDeviceInfo>
#include <QVector>

class FrameStore;

/// BluetoothManager handles Bluetooth connections to CAN adapters:
///   - ELM327 (OBD2 over Bluetooth SPP)
///   - ESP32 with CAN bus shield (custom GATT service)
///   - Generic Bluetooth SPP adapters
///
/// Connection flow:
///   1. Scan for devices
///   2. Select device
///   3. Connect via RFCOMM (SPP)
///   4. Start CAN frame exchange
class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothManager(FrameStore *store, QObject *parent = nullptr);
    ~BluetoothManager();

    /// Start scanning for Bluetooth devices.
    void startScan();

    /// Stop scanning.
    void stopScan();

    /// Connect to a specific device by address.
    void connectToDevice(const QString &address);

    /// Disconnect from the current device.
    void disconnectDevice();

    /// Whether currently connected.
    bool isConnected() const;

    /// List of discovered devices.
    QVector<QBluetoothDeviceInfo> discoveredDevices() const;

    /// Send a CAN frame over Bluetooth.
    void sendFrame(uint32_t id, const QByteArray &data, bool extended = false);

signals:
    void deviceDiscovered(const QString &name, const QString &address);
    void scanFinished();
    void connected();
    void disconnected();
    void frameReceived(uint32_t id, const QByteArray &data, bool extended);
    void errorOccurred(const QString &message);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QBluetoothSocket::SocketError error);

private:
    void parseElm327Line(const QByteArray &line);
    void parseEsp32Frame(const QByteArray &data);

    QBluetoothDeviceDiscoveryAgent *mDiscoveryAgent;
    QBluetoothSocket               *mSocket;
    FrameStore                      *mStore;
    QByteArray                       mBuffer;
    bool                             mElm327Mode = false;
    bool                             mConnected = false;
};

#endif // BLUETOOTHMANAGER_H
