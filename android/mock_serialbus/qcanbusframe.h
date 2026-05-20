#ifndef QCANBUSFRAME_H
#define QCANBUSFRAME_H

#include <QByteArray>

class QCanBusFrame
{
public:
    enum FrameType {
        UnknownFrame = 0,
        DataFrame,
        ErrorFrame,
        RemoteRequestFrame,
        InvalidFrame
    };

    enum FrameError {
        NoError = 0,
        TransmissionTimeoutError,
        LostArbitrationError,
        LocalDeviceError,
        ControllerError,
        ProtocolError,
        DatagramTooLongError,
        UnknownError,
        AnyError,
        ProtocolViolationError,
        TransceiverError,
        MissingAcknowledgmentError,
        BusOffError,
        BusError,
        ControllerRestartError
    };

    QCanBusFrame() : m_frameId(0), m_type(DataFrame), m_extended(false), m_flexibleDataRate(false), m_bitrateSwitch(false), m_errorStateIndicator(false) {}
    QCanBusFrame(FrameType type) : m_frameId(0), m_type(type), m_extended(false), m_flexibleDataRate(false), m_bitrateSwitch(false), m_errorStateIndicator(false) {}
    QCanBusFrame(quint32 identifier, const QByteArray &data) : m_frameId(identifier), m_payload(data), m_type(DataFrame), m_extended(false), m_flexibleDataRate(false), m_bitrateSwitch(false), m_errorStateIndicator(false) {}

    quint32 frameId() const { return m_frameId; }
    void setFrameId(quint32 id) { m_frameId = id; }

    bool hasExtendedFrameFormat() const { return m_extended; }
    void setExtendedFrameFormat(bool isExtended) { m_extended = isExtended; }

    FrameType frameType() const { return m_type; }
    void setFrameType(FrameType type) { m_type = type; }

    QByteArray payload() const { return m_payload; }
    void setPayload(const QByteArray &data) { m_payload = data; }

    bool isValid() const { return m_type != InvalidFrame; }

    struct TimeStamp {
        TimeStamp(qint64 s = 0, qint64 u = 0) : m_seconds(s), m_microSeconds(u) {}
        qint64 seconds() const { return m_seconds; }
        qint64 microSeconds() const { return m_microSeconds; }
    private:
        qint64 m_seconds;
        qint64 m_microSeconds;
    };

    TimeStamp timeStamp() const { return m_timeStamp; }
    void setTimeStamp(TimeStamp ts) { m_timeStamp = ts; }

    bool hasFlexibleDataRateFormat() const { return m_flexibleDataRate; }
    void setFlexibleDataRateFormat(bool isFlexibleDataRate) { m_flexibleDataRate = isFlexibleDataRate; }

    bool hasBitrateSwitch() const { return m_bitrateSwitch; }
    void setBitrateSwitch(bool bitrateSwitch) { m_bitrateSwitch = bitrateSwitch; }

    bool hasErrorStateIndicator() const { return m_errorStateIndicator; }
    void setErrorStateIndicator(bool errorStateIndicator) { m_errorStateIndicator = errorStateIndicator; }

    FrameError error() const { return NoError; }

private:
    quint32 m_frameId;
    QByteArray m_payload;
    FrameType m_type;
    bool m_extended;
    bool m_flexibleDataRate;
    bool m_bitrateSwitch;
    bool m_errorStateIndicator;
    TimeStamp m_timeStamp;
};

#endif // QCANBUSFRAME_H
