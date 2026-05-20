#include "BluetoothManager.h"
#include "can_structs.h"
#include <QDebug>
#include <QDateTime>
#include <QJniEnvironment>
#include <QJniObject>

// SPP UUID for standard Serial Port Profile
static const char *SPP_UUID = "00001101-0000-1000-8000-00805F9B34FB";

BluetoothManager::BluetoothManager(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    // Attempt to obtain the BluetoothAdapter early
    ensureAdapter();

    // Poll for already-bonded devices on a timer
    mPollTimer = new QTimer(this);
    connect(mPollTimer, &QTimer::timeout, this, &BluetoothManager::pollBondedDevices);

    // Read timer polls the Bluetooth socket for incoming data
    mReadTimer = new QTimer(this);
    connect(mReadTimer, &QTimer::timeout, this, &BluetoothManager::onReadTimer);
}

BluetoothManager::~BluetoothManager()
{
    disconnectDevice();
}

bool BluetoothManager::ensureAdapter()
{
    if (mBluetoothAdapter.isValid())
        return true;

    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");

    if (!activity.isValid()) {
        qWarning() << "BT: no Qt activity";
        return false;
    }

    QJniObject serviceName = QJniObject::fromString("bluetooth");
    QJniObject service = activity.callObjectMethod(
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
        serviceName.object<jstring>());

    if (!service.isValid()) {
        qWarning() << "BT: BluetoothManager service unavailable";
        return false;
    }

    mBluetoothAdapter = service.callObjectMethod(
        "getAdapter", "()Landroid/bluetooth/BluetoothAdapter;");

    if (!mBluetoothAdapter.isValid()) {
        qWarning() << "BT: no BluetoothAdapter (device may lack BT hardware)";
        return false;
    }

    bool enabled = mBluetoothAdapter.callMethod<jboolean>("isEnabled");
    qDebug() << "BT: adapter obtained, enabled:" << enabled;
    return enabled;
}

void BluetoothManager::startScan()
{
    if (!ensureAdapter()) {
        emit errorOccurred("Bluetooth not available");
        return;
    }

    // Android 12+ requires BLUETOOTH_SCAN permission + location
    mScanning = true;
    emit scanningChanged(true);

    // First emit already-paired/bonded devices for quick access
    pollBondedDevices();

    // Stop scan after 12 seconds (Android default discovery duration)
    QTimer::singleShot(12000, this, [this]() {
        if (mScanning) {
            mScanning = false;
            emit scanningChanged(false);
            emit scanFinished();
        }
    });
}

void BluetoothManager::stopScan()
{
    mScanning = false;
    emit scanningChanged(false);
}

void BluetoothManager::pollBondedDevices()
{
    if (!ensureAdapter()) return;

    QJniObject bondedSet = mBluetoothAdapter.callObjectMethod(
        "getBondedDevices", "()Ljava/util/Set;");

    if (!bondedSet.isValid()) return;

    QJniObject iterator = bondedSet.callObjectMethod(
        "iterator", "()Ljava/util/Iterator;");

    while (iterator.callMethod<jboolean>("hasNext")) {
        QJniObject device = iterator.callObjectMethod(
            "next", "()Ljava/lang/Object;");

        QJniObject name = device.callObjectMethod(
            "getName", "()Ljava/lang/String;");
        QJniObject addr = device.callObjectMethod(
            "getAddress", "()Ljava/lang/String;");

        emit deviceFound(name.toString(), addr.toString());
    }
}

void BluetoothManager::connectToDevice(const QString &address)
{
    if (!ensureAdapter()) {
        emit errorOccurred("Bluetooth not available");
        return;
    }

    // Disconnect any existing socket
    disconnectDevice();

    // Get the remote device by address
    QJniObject addrStr = QJniObject::fromString(address);
    QJniObject remoteDevice = mBluetoothAdapter.callObjectMethod(
        "getRemoteDevice", "(Ljava/lang/String;)Landroid/bluetooth/BluetoothDevice;",
        addrStr.object<jstring>());

    if (!remoteDevice.isValid()) {
        emit errorOccurred("Cannot find device: " + address);
        return;
    }

    // Create RFCOMM socket using SPP UUID
    QJniObject uuidStr = QJniObject::fromString(SPP_UUID);
    QJniObject uuid = QJniObject("java/util/UUID",
        "(Ljava/lang/String;)Ljava/util/UUID;",
        uuidStr.object<jstring>());

    mBluetoothSocket = remoteDevice.callObjectMethod(
        "createRfcommSocketToServiceRecord",
        "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothSocket;",
        uuid.object());

    if (!mBluetoothSocket.isValid()) {
        emit errorOccurred("Failed to create RFCOMM socket");
        return;
    }

    // Connect (blocking on Android, but quick for RFCOMM)
    mBluetoothSocket.callMethod<void>("connect");

    if (!mBluetoothSocket.callMethod<jboolean>("isConnected")) {
        emit errorOccurred("Failed to connect Bluetooth socket");
        closeSocket();
        return;
    }

    // Get I/O streams
    mInputStream = mBluetoothSocket.callObjectMethod(
        "getInputStream", "()Ljava/io/InputStream;");
    mOutputStream = mBluetoothSocket.callObjectMethod(
        "getOutputStream", "()Ljava/io/OutputStream;");

    mConnected = true;
    emit connectedChanged(true);
    mReadTimer->start(50); // poll every 50ms
    qDebug() << "BT: connected to" << address;
}

void BluetoothManager::disconnectDevice()
{
    mReadTimer->stop();
    closeSocket();
    mConnected = false;
    emit connectedChanged(false);
}

void BluetoothManager::closeSocket()
{
    if (mInputStream.isValid()) {
        mInputStream.callMethod<void>("close");
        mInputStream = QJniObject();
    }
    if (mOutputStream.isValid()) {
        mOutputStream.callMethod<void>("close");
        mOutputStream = QJniObject();
    }
    if (mBluetoothSocket.isValid()) {
        mBluetoothSocket.callMethod<void>("close");
        mBluetoothSocket = QJniObject();
    }
}

bool BluetoothManager::isConnected() const
{
    return mConnected;
}

bool BluetoothManager::isScanning() const
{
    return mScanning;
}

void BluetoothManager::sendFrame(uint32_t id, const QByteArray &data, bool extended)
{
    if (!mConnected || !mOutputStream.isValid()) return;

    // Serialize as SLCAN format: t#ID#DATA\r\n
    // (this is the format ESP32 CAN bridges typically use)
    QByteArray line;
    line += extended ? 'T' : 't';
    line += QByteArray::number(extended ? id : (id & 0x7FF), 16).rightJustified(extended ? 8 : 3, '0').toUpper();
    line += data.toHex().toUpper();
    line += '\r';

    // Write via JNI OutputStream.write(byte[], int, int)
    QJniEnvironment env;
    jbyteArray arr = env->NewByteArray(line.size());
    env->SetByteArrayRegion(arr, 0, line.size(),
        reinterpret_cast<const jbyte *>(line.constData()));

    mOutputStream.callMethod<void>("write", "([BII)V", arr, 0, line.size());
    mOutputStream.callMethod<void>("flush");

    env->DeleteLocalRef(arr);
}

void BluetoothManager::onReadTimer()
{
    if (!mConnected || !mInputStream.isValid()) return;

    QJniEnvironment env;
    jbyteArray arr = env->NewByteArray(256);

    jint bytesRead = mInputStream.callMethod<jint>(
        "read", "([B)I", arr);

    if (bytesRead > 0) {
        QByteArray chunk(bytesRead, Qt::Uninitialized);
        env->GetByteArrayRegion(arr, 0, bytesRead,
            reinterpret_cast<jbyte *>(chunk.data()));
        mReadBuffer.append(chunk);

        // Parse complete lines (terminated by \r or \n)
        while (true) {
            int nlIdx = mReadBuffer.indexOf('\r');
            if (nlIdx < 0) nlIdx = mReadBuffer.indexOf('\n');
            if (nlIdx < 0) break;

            QByteArray line = mReadBuffer.left(nlIdx).trimmed();
            mReadBuffer.remove(0, nlIdx + 1);

            if (line.isEmpty()) continue;
            if (!mStore) continue;

            // Parse SLCAN: t#ID#DATA or T#ID#DATA...
            bool isExt = line.startsWith('T');
            if (!isExt && !line.startsWith('t')) continue;

            int hash1 = line.indexOf('#', 1);
            if (hash1 < 0) continue;
            int hash2 = line.indexOf('#', hash1 + 1);
            if (hash2 < 0) continue;

            bool ok;
            QByteArray idPart = line.mid(1, hash1 - 1);
            uint32_t frameId = idPart.toUInt(&ok, 16);
            if (!ok) continue;

            QByteArray dataPart = line.mid(hash1 + 1, hash2 - hash1 - 1);
            QByteArray frameData = QByteArray::fromHex(dataPart);

            CANFrame f;
            f.bus = 0;
            f.setFrameId(frameId);
            f.setExtendedFrameFormat(isExt);
            f.setPayload(frameData);
            f.setTimeStamp(QCanBusFrame::TimeStamp(0, QDateTime::currentMSecsSinceEpoch() * 1000));
            f.isReceived = true;

            mStore->addFrame(f);
            emit frameReceived(frameId, frameData, isExt);
        }
    }

    env->DeleteLocalRef(arr);
}
