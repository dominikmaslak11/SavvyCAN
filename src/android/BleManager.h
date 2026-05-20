#ifndef BLEMANAGER_H
#define BLEMANAGER_H

#include <QObject>
#include <QJniObject>
#include <QTimer>
#include <QByteArray>

class FrameStore;

class BleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected   READ isConnected   NOTIFY connectedChanged)
    Q_PROPERTY(bool scanning    READ isScanning    NOTIFY scanningChanged)
    Q_PROPERTY(bool ledOn       READ isLedOn       NOTIFY ledStateChanged)

public:
    explicit BleManager(FrameStore *store = nullptr, QObject *parent = nullptr);
    ~BleManager();

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan();
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void writeLed(bool on);
    Q_INVOKABLE void readLed();

    // CAN
    Q_INVOKABLE void sendCanFrame(uint32_t id, bool extended, const QByteArray &data);

    bool isConnected() const { return mConnected; }
    bool isScanning() const  { return mScanning; }
    bool isLedOn() const     { return mLedOn; }

signals:
    void deviceFound(const QString &name, const QString &address, int rssi);
    void scanFinished();
    void connectedChanged(bool connected);
    void scanningChanged(bool scanning);
    void ledStateChanged(bool on);
    void canFrameReceived(uint32_t id, bool extended, const QByteArray &data);
    void errorOccurred(const QString &message);

private:
    void ensureBleHelper();

public:  // wewnętrzne sloty wołane z JNI przez invokeMethod
    void onDeviceFoundInternal(const QString &name, const QString &address, int rssi);
    void onScanFinishedInternal();
    void onConnectedInternal(const QString &address);
    void onDisconnectedInternal();
    void onLedStateChangedInternal(int state);
    void onCanFrameReceivedInternal(const QByteArray &frameData);
    void onErrorInternal(const QString &message);

    QJniObject mBleHelper;
    FrameStore *mStore = nullptr;
    bool       mConnected = false;
    bool       mScanning  = false;
    bool       mLedOn     = false;
    QTimer    *mScanTimer = nullptr;
};

#endif // BLEMANAGER_H
