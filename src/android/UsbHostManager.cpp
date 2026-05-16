#include "UsbHostManager.h"
#include "framestore.h"
#include <QDebug>
#include <QAndroidJniEnvironment>

UsbHostManager::UsbHostManager(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    // Get Android USB Manager service
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");

    QAndroidJniObject serviceName = QAndroidJniObject::fromString("usb");
    mUsbManager = activity.callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jstring>());
}

UsbHostManager::~UsbHostManager()
{
    closeDevice();
}

void UsbHostManager::enumerateDevices()
{
    if (!mUsbManager.isValid()) {
        emit errorOccurred("USB Manager not available");
        return;
    }

    // Call UsbManager.getDeviceList()
    QAndroidJniObject deviceMap = mUsbManager.callObjectMethod(
        "getDeviceList", "()Ljava/util/HashMap;");

    if (!deviceMap.isValid()) {
        emit errorOccurred("No USB devices found");
        return;
    }

    // Get key set (device names)
    QAndroidJniObject keySet = deviceMap.callObjectMethod(
        "keySet", "()Ljava/util/Set;");
    QAndroidJniObject iterator = keySet.callObjectMethod(
        "iterator", "()Ljava/util/Iterator;");

    // Known PEAK PCAN USB VID/PID pairs
    struct PcanDevice { int vid; int pid; const char *name; };
    static const PcanDevice knownDevices[] = {
        {0x0C72, 0x000C, "PEAK PCAN-USB"},
        {0x0C72, 0x001C, "PEAK PCAN-USB FD"},
        {0x0C72, 0x000D, "PEAK PCAN-USB Pro"},
        {0x0C72, 0x001D, "PEAK PCAN-USB Pro FD"},
    };

    while (iterator.callMethod<jboolean>("hasNext")) {
        QAndroidJniObject deviceName = iterator.callObjectMethod("next", "()Ljava/lang/Object;");
        QString name = deviceName.toString();
        emit deviceFound(name, 0, 0);
    }

    // Also emit known supported devices for quick access
    for (const auto &dev : knownDevices) {
        emit deviceFound(QString("(known) %1 [%2:%3]").arg(dev.name).arg(dev.vid, 4, 16).arg(dev.pid, 4, 16),
                        dev.vid, dev.pid);
    }
}

void UsbHostManager::openDevice(int vendorId, int productId)
{
    if (!mUsbManager.isValid()) {
        emit errorOccurred("USB Manager not available");
        return;
    }

    // Build USB device filter
    QAndroidJniObject filter = QAndroidJniObject("android/hardware/usb/UsbDevice",
        "(II)Z", vendorId, productId);

    // Request permission via PendingIntent
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");

    QAndroidJniObject intent("android/content/Intent",
        "(Ljava/lang/String;)V", QAndroidJniObject::fromString("com.savvycan.USB_PERMISSION").object<jstring>());

    QAndroidJniObject pendingIntent = activity.callObjectMethod(
        "createPendingResult",
        "(ILandroid/content/Intent;I)Landroid/app/PendingIntent;",
        0, intent.object(), 0);

    mUsbManager.callMethod<void>("requestPermission",
        "(Landroid/hardware/usb/UsbDevice;Landroid/app/PendingIntent;)V",
        filter.object(), pendingIntent.object());

    // Open device
    QAndroidJniObject device = mUsbManager.callObjectMethod(
        "openDevice", "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;",
        filter.object());

    if (!device.isValid()) {
        emit errorOccurred("Failed to open USB device");
        return;
    }

    mUsbDevice = filter;
    mUsbConnection = device;
    mOpen = true;

    emit deviceOpened(QString("USB %1:%2").arg(vendorId, 4, 16).arg(productId, 4, 16));
}

void UsbHostManager::closeDevice()
{
    if (mUsbConnection.isValid()) {
        mUsbConnection.callMethod<void>("close");
    }
    mOpen = false;
    emit deviceClosed();
}

bool UsbHostManager::isOpen() const
{
    return mOpen;
}

bool UsbHostManager::writeBulk(const QByteArray &data)
{
    if (!mOpen || !mUsbConnection.isValid()) return false;

    QAndroidJniEnvironment env;
    jbyteArray arr = env->NewByteArray(data.size());
    env->SetByteArrayRegion(arr, 0, data.size(),
        reinterpret_cast<const jbyte *>(data.constData()));

    jint result = mUsbConnection.callMethod<jint>(
        "bulkTransfer",
        "(Landroid/hardware/usb/UsbEndpoint;[BII)I",
        mUsbEndpointOut.object(), arr, data.size(), 1000);

    env->DeleteLocalRef(arr);
    return result >= 0;
}

QByteArray UsbHostManager::readBulk(int maxSize)
{
    if (!mOpen || !mUsbConnection.isValid()) return {};

    QAndroidJniEnvironment env;
    jbyteArray arr = env->NewByteArray(maxSize);

    jint result = mUsbConnection.callMethod<jint>(
        "bulkTransfer",
        "(Landroid/hardware/usb/UsbEndpoint;[BII)I",
        mUsbEndpointIn.object(), arr, maxSize, 1000);

    if (result <= 0) {
        env->DeleteLocalRef(arr);
        return {};
    }

    QByteArray data(result, Qt::Uninitialized);
    env->GetByteArrayRegion(arr, 0, result,
        reinterpret_cast<jbyte *>(data.data()));
    env->DeleteLocalRef(arr);

    return data;
}

void UsbHostManager::sendFrame(uint32_t id, const QByteArray &data, bool extended)
{
    if (!mOpen) return;

    // PCAN USB protocol (simplified)
    // Frame: [type 1B][flags 1B][id 4B][dlc 1B][data 8B]
    QByteArray frame;
    frame.append('\x02');  // Message type: CAN frame
    frame.append(extended ? '\x80' : '\x00');  // Flags: extended
    frame.append(static_cast<char>(id & 0xFF));
    frame.append(static_cast<char>((id >> 8) & 0xFF));
    frame.append(static_cast<char>((id >> 16) & 0xFF));
    frame.append(static_cast<char>((id >> 24) & 0xFF));
    frame.append(static_cast<char>(data.size()));
    frame.append(data.left(8));
    while (frame.size() < 16)
        frame.append('\x00');

    writeBulk(frame);
}
