#ifndef LIN_CAN_BRIDGE_H
#define LIN_CAN_BRIDGE_H

#include "connections/canconnection.h"
#include "lin/lin_connection.h"
#include "lin/lin_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// LinCanBridge — wraps a LINConnection as a CANConnection so it appears
// in the existing connection management UI and frame pipeline.
//
// LIN frames are converted to CAN frames for display:
//   - CAN ID = LIN PID (8-bit, stored in standard 11-bit CAN ID)
//   - CAN DLC = LIN dataLen
//   - CAN data = LIN data bytes
//   - bus field distinguishes LIN buses from CAN buses
// ═══════════════════════════════════════════════════════════════════════════

class LinCanBridge : public CANConnection
{
    Q_OBJECT

public:
    explicit LinCanBridge(LINConnection *linConn,
                          const QString &port,
                          int linBaudRate = LIN::BAUD_19200,
                          int numBuses = 1);

    virtual ~LinCanBridge();

    /// Direct access to the underlying LIN connection
    LINConnection* linConnection() const { return mLinConn; }

    /// Override to return the correct LIN type
    CANCon::type getType() const noexcept override;

protected:
    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame &frame) override;
    bool piSendFrames(const QList<CANFrame> &frames) override;

private slots:
    void onLinFramesReceived(int bus, QVector<LINFrame> frames);
    void onLinStatusChanged(LINCon::status s);
    void onLinError(const QString &msg);

private:
    CANFrame linFrameToCanFrame(const LINFrame &lin);
    LINFrame canFrameToLinFrame(const CANFrame &can);

    LINConnection *mLinConn = nullptr;
    int mLinBaudRate;
    CANCon::type mActualType = CANCon::LIN_SERIAL;
};

#endif // LIN_CAN_BRIDGE_H
