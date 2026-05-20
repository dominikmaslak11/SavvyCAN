#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <QJniObject>
#include <QJniEnvironment>
#include <QTimer>
#include <QByteArray>
#include "framestore.h"

/// BluetoothManager handles Bluetooth Classic (SPP) CAN bridge connections on Android.
///
/// Uses JNI to call Android's BluetoothAdapter / BluetoothSocket APIs.
/// Connects to ESP32 CAN bridges, ELM327 adapters, and other SPP-based CAN devices.
class BluetoothManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)

public:
    explicit BluetoothManager(FrameStore *store, QObject *parent = nullptr);
    ~BluetoothManager();

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE bool isConnected() const;
    Q_INVOKABLE bool isScanning() const;
    Q_INVOKABLE void sendFrame(uint32_t id, const QByteArray &data, bool extended);

signals:
    void deviceFound(const QString &name, const QString &address);
    void scanFinished();
    void connectedChanged(bool connected);
    void scanningChanged(bool scanning);
    void errorOccurred(const QString &errorMsg);
    void frameReceived(uint32_t id, const QByteArray &data, bool extended);

private slots:
    void pollBondedDevices();
    void onReadTimer();

private:
    bool ensureAdapter();
    void closeSocket();

    FrameStore *mStore = nullptr;
    QJniObject  mBluetoothAdapter;
    QJniObject  mBluetoothSocket;
    QJniObject  mInputStream;
    QJniObject  mOutputStream;
    QTimer     *mPollTimer = nullptr;
    QTimer     *mReadTimer = nullptr;
    QByteArray  mReadBuffer;
    bool        mConnected = false;
    bool        mScanning = false;
};

#endif // BLUETOOTHMANAGER_H
