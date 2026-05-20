#include "qcanbusdevice.h"
#include "qcanbus.h"

// ── QCanBus ───────────────────────────────────────────────────────────────

QCanBus *QCanBus::self = nullptr;

QCanBus::QCanBus(QObject *parent) : QObject(parent) {}

QCanBus *QCanBus::instance()
{
    if (!self) self = new QCanBus();
    return self;
}

QList<QCanBusDeviceInfo> QCanBus::availableDevices(const QString &, QString *) const
{
    return {}; // No devices on Android via QtSerialBus
}

QCanBusDevice *QCanBus::createDevice(const QString &, const QString &, QString *)
{
    return nullptr;
}

QStringList QCanBus::plugins() const
{
    return {}; // No serialbus plugins on Android
}

// ── QCanBusDevice ─────────────────────────────────────────────────────────

QCanBusDevice::QCanBusDevice(QObject *parent) : QObject(parent) {}
QCanBusDevice::~QCanBusDevice() = default;

bool QCanBusDevice::connectDevice() { setState(ConnectedState); return true; }
void QCanBusDevice::disconnectDevice() { setState(UnconnectedState); }
bool QCanBusDevice::writeFrame(const QCanBusFrame &) { return false; }

QCanBusDevice::CanBusDeviceState QCanBusDevice::state() const { return m_state; }
QCanBusDevice::CanBusError QCanBusDevice::error() const { return m_error; }
QString QCanBusDevice::errorString() const { return m_errorString; }

QString QCanBusDevice::interpretErrorFrame(const QCanBusFrame &) { return {}; }

QVector<QCanBusFrame> QCanBusDevice::framesToWrite() const { return m_outgoingFrames; }
qint64 QCanBusDevice::framesWritten() const { return m_framesWritten; }
QVector<QCanBusFrame> QCanBusDevice::framesAvailable() const { return m_incomingFrames; }

bool QCanBusDevice::hasBusStatus() const { return false; }
QCanBusDevice::CanBusStatus QCanBusDevice::busStatus() const { return UnknownStatus; }
QCanBusDeviceInfo QCanBusDevice::deviceInfo() const { return {}; }

bool QCanBusDevice::setConfigurationParameter(int, const QVariant &) { return false; }
QVariant QCanBusDevice::configurationParameter(int) const { return {}; }
QVector<int> QCanBusDevice::configurationKeys() const { return {}; }

void QCanBusDevice::clear(Directions) {}

void QCanBusDevice::setState(CanBusDeviceState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}

void QCanBusDevice::setError(const QString &errorText, CanBusError errorId)
{
    m_errorString = errorText;
    m_error = errorId;
    emit errorOccurred(errorId);
}

void QCanBusDevice::enqueueReceivedFrames(const QVector<QCanBusFrame> &frames)
{
    m_incomingFrames.append(frames);
    if (!frames.isEmpty()) emit framesReceived();
}

void QCanBusDevice::enqueueOutgoingFrames(const QVector<QCanBusFrame> &frames)
{
    m_outgoingFrames.append(frames);
}

void QCanBusDevice::processOutgoingFrames() {}
void QCanBusDevice::processIncomingFrames() {}
