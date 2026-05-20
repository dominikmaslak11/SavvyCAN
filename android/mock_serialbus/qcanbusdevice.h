#ifndef QCANBUSDEVICE_H
#define QCANBUSDEVICE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QVariant>
#include "qcanbusframe.h"

class QCanBusDeviceInfo
{
public:
    QCanBusDeviceInfo() = default;
    QCanBusDeviceInfo(const QString &name, const QString &description = QString(),
                       const QString &serial = QString(), int channel = -1)
        : m_name(name), m_description(description), m_serial(serial), m_channel(channel) {}
    
    QString name() const { return m_name; }
    QString description() const { return m_description; }
    QString serialNumber() const { return m_serial; }
    int channel() const { return m_channel; }
    bool isVirtual() const { return false; }

private:
    QString m_name;
    QString m_description;
    QString m_serial;
    int m_channel = -1;
};

class QCanBusDevice : public QObject
{
    Q_OBJECT
public:
    enum CanBusError {
        NoError = 0,
        ReadError,
        WriteError,
        ConnectionError,
        ConfigurationError,
        UnknownError,
        OperationError,
        TimeoutError
    };
    Q_ENUM(CanBusError)

    enum CanBusDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };
    Q_ENUM(CanBusDeviceState)
    typedef CanBusDeviceState QCanBusDeviceState;

    enum CanBusStatus {
        UnknownStatus = 0,
        GoodStatus,
        WarningStatus,
        ErrorStatus,
        BusOffStatus
    };

    enum ConfigurationKey {
        RawFilterKey = 0,
        ErrorFilterKey,
        LoopbackKey,
        ReceiveOwnKey,
        BitRateKey,
        CanFdKey,
        DataBitRateKey,
        ProtocolKey,
        UserKey
    };

    enum Direction { Input = 1, Output = 2, AllDirections = Input | Output };
    typedef Direction Directions;

    explicit QCanBusDevice(QObject *parent = nullptr);
    virtual ~QCanBusDevice();

    virtual bool connectDevice();
    virtual void disconnectDevice();
    virtual bool writeFrame(const QCanBusFrame &frame);

    CanBusDeviceState state() const;
    CanBusError error() const;
    QString errorString() const;

    virtual QString interpretErrorFrame(const QCanBusFrame &errorFrame);

    QVector<QCanBusFrame> framesToWrite() const;
    qint64 framesWritten() const;
    QVector<QCanBusFrame> framesAvailable() const;

    bool hasBusStatus() const;
    CanBusStatus busStatus() const;
    
    QCanBusDeviceInfo deviceInfo() const;

    virtual bool setConfigurationParameter(int key, const QVariant &value);
    virtual QVariant configurationParameter(int key) const;
    QVector<int> configurationKeys() const;

    virtual void clear(Directions direction);

signals:
    void errorOccurred(CanBusError);
    void framesReceived();
    void framesWritten(qint64 count);
    void stateChanged(CanBusDeviceState state);

protected:
    void setState(CanBusDeviceState state);
    void setError(const QString &errorText, CanBusError errorId);
    void enqueueReceivedFrames(const QVector<QCanBusFrame> &frames);
    void enqueueOutgoingFrames(const QVector<QCanBusFrame> &frames);

    virtual void processOutgoingFrames();
    virtual void processIncomingFrames();

private:
    CanBusDeviceState m_state = UnconnectedState;
    CanBusError m_error = NoError;
    QString m_errorString;
    QVector<QCanBusFrame> m_incomingFrames;
    QVector<QCanBusFrame> m_outgoingFrames;
    qint64 m_framesWritten = 0;
};

Q_DECLARE_METATYPE(QCanBusDevice::CanBusDeviceState)

#endif // QCANBUSDEVICE_H
