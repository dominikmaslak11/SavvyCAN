#include "lin_can_bridge.h"
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════════════
// LinCanBridge implementation
// ═══════════════════════════════════════════════════════════════════════════

LinCanBridge::LinCanBridge(LINConnection *linConn,
                           const QString &port,
                           int linBaudRate,
                           int numBuses)
    : CANConnection(port,
                    "LIN",
                    CANCon::LIN_SERIAL,  // default; override per actual type
                    linBaudRate,
                    linBaudRate,
                    false,   // no CAN FD
                    0,       // no separate data rate
                    numBuses,
                    1024,    // queue length
                    false)   // no dedicated thread (LIN thread handles it)
    , mLinConn(linConn)
    , mLinBaudRate(linBaudRate)
{
    if (!mLinConn) return;

    // Determine CANCon::type from LINCon::type
    switch (mLinConn->getType()) {
    case LINCon::LIN_SOCKETCAN: mActualType = CANCon::LIN_SOCKETCAN; break;
    case LINCon::LIN_SERIAL:
    default:                    mActualType = CANCon::LIN_SERIAL; break;
    }

    // Wire up LIN signals to internal slots
    QObject::connect(mLinConn, &LINConnection::framesReceived,
                     this, &LinCanBridge::onLinFramesReceived);
    QObject::connect(mLinConn, &LINConnection::statusChanged,
                     this, &LinCanBridge::onLinStatusChanged);
    QObject::connect(mLinConn, &LINConnection::error,
                     this, &LinCanBridge::onLinError);
}

LinCanBridge::~LinCanBridge()
{
    if (mLinConn) {
        mLinConn->stop();
    }
}

// ── Protetected interface (CANConnection overrides) ──────────────────────

void LinCanBridge::piStarted()
{
    if (mLinConn)
        mLinConn->start();
}

void LinCanBridge::piStop()
{
    if (mLinConn)
        mLinConn->stop();
}

void LinCanBridge::piSetBusSettings(int pBusIdx, CANBus pBus)
{
    Q_UNUSED(pBusIdx)
    Q_UNUSED(pBus)
    // LIN bus settings are set separately
}

bool LinCanBridge::piGetBusSettings(int pBusIdx, CANBus &pBus)
{
    Q_UNUSED(pBusIdx)
    Q_UNUSED(pBus)
    return true;
}

void LinCanBridge::piSuspend(bool pSuspend)
{
    if (mLinConn)
        mLinConn->suspend(pSuspend);
}

bool LinCanBridge::piSendFrame(const CANFrame &frame)
{
    if (!mLinConn) return false;

    LINFrame lin = canFrameToLinFrame(frame);
    return mLinConn->sendFrame(lin);
}

bool LinCanBridge::piSendFrames(const QList<CANFrame> &frames)
{
    if (!mLinConn) return false;

    bool allOk = true;
    for (const auto &can : frames) {
        LINFrame lin = canFrameToLinFrame(can);
        if (!mLinConn->sendFrame(lin))
            allOk = false;
    }
    return allOk;
}

// ── LIN signal handlers ──────────────────────────────────────────────────

void LinCanBridge::onLinFramesReceived(int bus, QVector<LINFrame> frames)
{
    for (const auto &lin : frames) {
        CANFrame can = linFrameToCanFrame(lin);
        can.bus = bus;
        CANFrame* frame_p = getQueue().get();
        if (frame_p) {
            *frame_p = can;
            getQueue().queue();
        }
        checkTargettedFrame(can);
    }
}

void LinCanBridge::onLinStatusChanged(LINCon::status s)
{
    if (s == LINCon::CONNECTED)
        setStatus(CANCon::CONNECTED);
    else
        setStatus(CANCon::NOT_CONNECTED);
}

void LinCanBridge::onLinError(const QString &msg)
{
    emit error(msg);
}

// ── Conversion helpers ───────────────────────────────────────────────────

CANFrame LinCanBridge::linFrameToCanFrame(const LINFrame &lin)
{
    CANFrame can;
    // Use LIN PID as CAN ID (8-bit, fits in 11-bit standard ID)
    can.setFrameId(lin.pid);
    can.setExtendedFrameFormat(false);
    can.setFrameType(QCanBusFrame::DataFrame);

    QByteArray payload;
    payload.resize(lin.dataLen);
    for (int i = 0; i < lin.dataLen; ++i)
        payload[i] = (char)lin.data[i];
    can.setPayload(payload);

    // We encode LIN-specific metadata in the CAN frame's timestamp
    // The actual timestamp comes from the LIN frame
    can.setTimeStamp(QCanBusFrame::TimeStamp(0, lin.timestamp));

    can.isReceived = lin.isReceived;
    can.bus = lin.bus;

    return can;
}

LINFrame LinCanBridge::canFrameToLinFrame(const CANFrame &can)
{
    LINFrame lin;
    lin.pid = (uint8_t)(can.frameId() & 0xFF);
    lin.id = LINHelpers::pidToID(lin.pid);
    lin.bus = can.bus;
    lin.isReceived = can.isReceived;

    QByteArray payload = can.payload();
    lin.dataLen = (uint8_t)qMin(payload.size(), LIN::MAX_DATA_LEN);
    for (int i = 0; i < lin.dataLen; ++i)
        lin.data[i] = (uint8_t)payload[i];

    // Build checksum
    lin.checksumType = LINChecksumType::Enhanced;
    lin.checksum = LINHelpers::calcEnhancedChecksum(lin.pid, lin.data, lin.dataLen);
    lin.calcChecksum = lin.checksum;

    // Determine frame type from ID
    LINHelpers::frameTypeFromID(lin.id);

    return lin;
}

// ── getType override ─────────────────────────────────────────────────────

CANCon::type LinCanBridge::getType() const noexcept
{
    return mActualType;
}
