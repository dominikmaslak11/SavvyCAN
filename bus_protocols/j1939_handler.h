#ifndef J1939_HANDLER_H
#define J1939_HANDLER_H

#include <Qt>
#include <QObject>
#include <QMap>
#include <QVector>
#include <QElapsedTimer>
#include "can_structs.h"
#include "j1939_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// J1939Handler — decodes CAN frames into J1939 PGNs/SPNs/DTCs
// Provides real-time multi-frame reassembly via J1939 Transport Protocol
// ═══════════════════════════════════════════════════════════════════════════

class J1939Handler : public QObject
{
    Q_OBJECT

public:
    explicit J1939Handler(QObject *parent = nullptr);

    /// Process a CAN frame — returns true if a complete J1939 message was decoded
    bool processFrame(const CANFrame &frame);

    /// Get the last decoded PGN
    uint32_t lastPGN() const { return mLastPGN; }

    /// Get the J1939 database
    const J1939_DB& database() const { return mDB; }
    void setDatabase(const J1939_DB &db);

    /// Decode a raw 8-byte data array according to a specific PGN definition
    QMap<uint32_t, double> decodeSPNs(uint32_t pgn, const uint8_t *data, int len) const;

    /// Get active DTCs from DM1 messages
    QVector<J1939_DTC> activeDTCs() const { return mActiveDTCs; }

signals:
    void pgnDecoded(uint32_t pgn, QMap<uint32_t, double> spnValues);
    void dtcUpdated(const QVector<J1939_DTC> &dtcs);
    void addressClaimed(const J1939_AddressClaim &claim);
    void transportComplete(uint32_t pgn, const QByteArray &data);
    void debugMessage(const QString &msg);

private:
    // Transport Protocol reassembly state
    struct TPState {
        bool     active      = false;
        uint32_t pgn         = 0;
        uint8_t  srcAddr     = 0;
        uint8_t  totalPackets = 0;
        uint32_t totalBytes   = 0;
        uint8_t  nextPacket   = 0;
        QByteArray buffer;
        QElapsedTimer timer;
    };

    void handleTransportCM(const CANFrame &frame);
    void handleTransportDT(const CANFrame &frame);
    void handleAddressClaim(const CANFrame &frame);
    void handleDM1(const CANFrame &frame);
    void handleDM2(const CANFrame &frame);

    J1939_DB    mDB;
    uint32_t    mLastPGN = 0;
    QVector<J1939_DTC> mActiveDTCs;
    QVector<J1939_DTC> mPreviousDTCs;
    QMap<uint16_t, J1939_AddressClaim> mAddressClaims;  // keyed by source addr
    QMap<QPair<uint8_t, uint8_t>, TPState> mTPStates;   // (SA, PGN) -> state
};

#endif // J1939_HANDLER_H
