#include "BleManager.h"
#include "framestore.h"
#include "can_structs.h"
#include <QDebug>
#include <QJniEnvironment>
#include <QJniObject>
#include <QDateTime>
#include <jni.h>

static BleManager *gBleManager = nullptr;

// ═══════════════════════════════════════════════════════════════════════════
// JNI native methods
// ═══════════════════════════════════════════════════════════════════════════

extern "C" {

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onDeviceFound(
        JNIEnv *env, jclass, jstring name, jstring address, jint rssi)
{
    if (!gBleManager) return;
    const char *n = env->GetStringUTFChars(name, nullptr);
    const char *a = env->GetStringUTFChars(address, nullptr);
    QMetaObject::invokeMethod(gBleManager, [n = QString::fromUtf8(n),
        a = QString::fromUtf8(a), rssi]() {
        if (gBleManager) gBleManager->onDeviceFoundInternal(n, a, rssi);
    }, Qt::QueuedConnection);
    env->ReleaseStringUTFChars(name, n);
    env->ReleaseStringUTFChars(address, a);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onScanFinished(JNIEnv *, jclass)
{
    if (gBleManager) QMetaObject::invokeMethod(gBleManager, []() {
        if (gBleManager) gBleManager->onScanFinishedInternal();
    }, Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onConnected(JNIEnv *env, jclass, jstring address)
{
    if (!gBleManager) return;
    const char *a = env->GetStringUTFChars(address, nullptr);
    QMetaObject::invokeMethod(gBleManager, [a = QString::fromUtf8(a)]() {
        if (gBleManager) gBleManager->onConnectedInternal(a);
    }, Qt::QueuedConnection);
    env->ReleaseStringUTFChars(address, a);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onDisconnected(JNIEnv *, jclass)
{
    if (gBleManager) QMetaObject::invokeMethod(gBleManager, []() {
        if (gBleManager) gBleManager->onDisconnectedInternal();
    }, Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onLedStateChanged(JNIEnv *, jclass, jint state)
{
    if (gBleManager) QMetaObject::invokeMethod(gBleManager, [state]() {
        if (gBleManager) gBleManager->onLedStateChangedInternal(state);
    }, Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onCanFrameReceived(JNIEnv *env, jclass, jbyteArray data)
{
    if (!gBleManager) return;

    jsize len = env->GetArrayLength(data);
    jbyte *bytes = env->GetByteArrayElements(data, nullptr);

    QByteArray frameData((const char*)bytes, len);

    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);

    QMetaObject::invokeMethod(gBleManager, [frameData]() {
        if (gBleManager) gBleManager->onCanFrameReceivedInternal(frameData);
    }, Qt::QueuedConnection);
}

JNIEXPORT void JNICALL
Java_com_savvycan_android_BleHelper_onError(JNIEnv *env, jclass, jstring message)
{
    if (!gBleManager) return;
    const char *m = env->GetStringUTFChars(message, nullptr);
    QMetaObject::invokeMethod(gBleManager, [m = QString::fromUtf8(m)]() {
        if (gBleManager) gBleManager->onErrorInternal(m);
    }, Qt::QueuedConnection);
    env->ReleaseStringUTFChars(message, m);
}

} // extern "C"

// ═══════════════════════════════════════════════════════════════════════════
// BleManager
// ═══════════════════════════════════════════════════════════════════════════

BleManager::BleManager(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    gBleManager = this;
    mScanTimer = new QTimer(this);
    mScanTimer->setSingleShot(true);
    connect(mScanTimer, &QTimer::timeout, this, [this]() { stopScan(); });
    ensureBleHelper();
}

BleManager::~BleManager()
{
    if (mBleHelper.isValid())
        mBleHelper.callMethod<void>("disconnect");
    gBleManager = nullptr;
}

void BleManager::ensureBleHelper()
{
    if (mBleHelper.isValid()) return;

    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity",
        "()Landroid/app/Activity;");
    if (!activity.isValid()) { qWarning() << "BLE: no activity"; return; }

    QJniObject context = activity.callObjectMethod(
        "getApplicationContext", "()Landroid/content/Context;");
    if (!context.isValid()) { qWarning() << "BLE: no context"; return; }

    mBleHelper = QJniObject("com/savvycan/android/BleHelper",
        "(Landroid/content/Context;)V", context.object());
    if (mBleHelper.isValid())
        qDebug() << "BLE: BleHelper created";
    else
        qWarning() << "BLE: BleHelper creation failed";
}

// ═══════════════════════════════════════════════════════════════════════════
// Internal slots (called from JNI via invokeMethod)
// ═══════════════════════════════════════════════════════════════════════════

void BleManager::onDeviceFoundInternal(const QString &name, const QString &address, int rssi)
{ emit deviceFound(name, address, rssi); }

void BleManager::onScanFinishedInternal()
{ mScanning = false; emit scanningChanged(false); emit scanFinished(); }

void BleManager::onConnectedInternal(const QString &address)
{ mConnected = true; emit connectedChanged(true); qDebug() << "BLE: connected to" << address; }

void BleManager::onDisconnectedInternal()
{ mConnected = false; mLedOn = false; emit ledStateChanged(false); emit connectedChanged(false); qDebug() << "BLE: disconnected"; }

void BleManager::onLedStateChangedInternal(int state)
{ mLedOn = (state != 0); emit ledStateChanged(state != 0); }

void BleManager::onCanFrameReceivedInternal(const QByteArray &frameData)
{
    if (frameData.size() < 6) return;

    // Parsuj format binarny:
    // [0-3] ID little-endian, [4] DLC, [5] flags, [6+] data
    const uint8_t *d = (const uint8_t*)frameData.constData();
    uint32_t id = d[0] | ((uint32_t)d[1] << 8) | ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24);
    uint8_t dlc = d[4];
    bool extended = (d[5] & 0x01) != 0;
    if (dlc > 8) dlc = 8;

    int dataLen = frameData.size() - 6;
    if (dataLen < dlc) dlc = dataLen;

    QByteArray payload((const char*)(d + 6), dlc);

    emit canFrameReceived(id, extended, payload);
    qDebug() << "CAN RX: ID=" << Qt::hex << id << (extended ? "ext" : "std") << "DLC=" << dlc;

    if (mStore) {
        CANFrame f;
        f.bus = 0;
        f.setFrameId(id);
        f.setExtendedFrameFormat(extended);
        f.setPayload(payload);
        f.setTimeStamp(QCanBusFrame::TimeStamp(0, QDateTime::currentMSecsSinceEpoch() * 1000));
        f.isReceived = true;
        mStore->addFrame(f);
    }
}

void BleManager::onErrorInternal(const QString &message)
{ emit errorOccurred(message); qWarning() << "BLE error:" << message; }

// ═══════════════════════════════════════════════════════════════════════════
// Public operations (QML-callable)
// ═══════════════════════════════════════════════════════════════════════════

void BleManager::startScan()
{
    if (!mBleHelper.isValid()) { ensureBleHelper(); }
    if (!mBleHelper.isValid()) { emit errorOccurred("BLE not available"); return; }
    mBleHelper.callMethod<void>("startScan");
    mScanning = true; emit scanningChanged(true);
    mScanTimer->start(15000);
    qDebug() << "BLE: scan started";
}

void BleManager::stopScan()
{
    if (!mBleHelper.isValid()) return;
    mScanTimer->stop();
    mBleHelper.callMethod<void>("stopScan");
    mScanning = false; emit scanningChanged(false); emit scanFinished();
    qDebug() << "BLE: scan stopped";
}

void BleManager::connectToDevice(const QString &address)
{
    if (!mBleHelper.isValid()) { ensureBleHelper(); }
    if (!mBleHelper.isValid()) { emit errorOccurred("BLE not available"); return; }
    QJniObject addr = QJniObject::fromString(address);
    mBleHelper.callMethod<void>("connectToDevice", "(Ljava/lang/String;)V", addr.object<jstring>());
    qDebug() << "BLE: connecting to" << address;
}

void BleManager::disconnectDevice()
{ if (mBleHelper.isValid()) mBleHelper.callMethod<void>("disconnect"); }

void BleManager::writeLed(bool on)
{
    if (!mBleHelper.isValid()) { emit errorOccurred("BLE not available"); return; }
    mBleHelper.callMethod<void>("writeLed", "(Z)V", on);
    qDebug() << "BLE: writeLed" << (on ? "ON" : "OFF");
}

void BleManager::readLed()
{ if (mBleHelper.isValid()) mBleHelper.callMethod<void>("readLed"); }

void BleManager::sendCanFrame(uint32_t id, bool extended, const QByteArray &data)
{
    if (!mBleHelper.isValid()) { emit errorOccurred("BLE not available"); return; }

    QJniEnvironment env;
    int dlc = qMin(data.size(), 8);

    jbyteArray jdata = env->NewByteArray(dlc);
    env->SetByteArrayRegion(jdata, 0, dlc, (const jbyte*)data.constData());

    mBleHelper.callMethod<void>("sendCanFrame", "(IZI[B)V", (jint)id, (jboolean)extended, (jint)dlc, jdata);

    env->DeleteLocalRef(jdata);
    qDebug() << "CAN TX: ID=" << Qt::hex << id << (extended ? "ext" : "std") << "DLC=" << dlc;
}
