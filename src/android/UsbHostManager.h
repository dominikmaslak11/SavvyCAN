#ifndef USBHOSTMANAGER_H
#define USBHOSTMANAGER_H

#include <QObject>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QVector>

class FrameStore;

/// UsbHostManager provides USB OTG connectivity for PEAK PCAN USB adapters.
///
/// Uses Android's USB Host API (android.hardware.usb.UsbManager)
/// to detect and communicate with USB CAN adapters.
///
/// Supported devices:
///   - PEAK PCAN-USB (vendor 0x0C72, product 0x000C)
///   - PCAN-USB FD (vendor 0x0C72, product 0x001C)
class UsbHostManager : public QObject
{
    Q_OBJECT

public:
    explicit UsbHostManager(FrameStore *store, QObject *parent = nullptr);
    ~UsbHostManager();

    /// Enumerate connected USB devices.
    void enumerateDevices();

    /// Request permission and open the device.
    void openDevice(int vendorId, int productId);

    /// Close the USB device.
    void closeDevice();

    /// Whether a device is currently open.
    bool isOpen() const;

    /// Send a CAN frame over USB.
    void sendFrame(uint32_t id, const QByteArray &data, bool extended = false);

signals:
    void deviceFound(const QString &name, int vendorId, int productId);
    void deviceOpened(const QString &name);
    void deviceClosed();
    void frameReceived(uint32_t id, const QByteArray &data, bool extended);
    void errorOccurred(const QString &message);

private:
    bool writeBulk(const QByteArray &data);
    QByteArray readBulk(int maxSize = 64);

    FrameStore *mStore;
    QAndroidJniObject mUsbManager;
    QAndroidJniObject mUsbDevice;
    QAndroidJniObject mUsbConnection;
    QAndroidJniObject mUsbEndpointIn;
    QAndroidJniObject mUsbEndpointOut;
    bool mOpen = false;
};

#endif // USBHOSTMANAGER_H
