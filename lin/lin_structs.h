#ifndef LIN_STRUCTS_H
#define LIN_STRUCTS_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <stdint.h>
#include <QtMath>

// ═══════════════════════════════════════════════════════════════════════════
// LIN (Local Interconnect Network) data structures for SavvyCAN
// Based on LIN 2.2A specification
// ═══════════════════════════════════════════════════════════════════════════

// ── Protocol constants ────────────────────────────────────────────────────

namespace LIN {
    constexpr int MAX_DATA_LEN        = 8;
    constexpr int BREAK_MIN_BITS      = 13;
    constexpr int SYNC_BYTE           = 0x55;
    constexpr int PID_MASK_ID         = 0x3F;  // 6-bit ID
    constexpr int PID_MASK_PARITY     = 0xC0;  // 2-bit parity
    constexpr int MIN_ID              = 0;
    constexpr int MAX_ID              = 63;
    constexpr int DIAG_MASTER_REQ_ID  = 60;
    constexpr int DIAG_SLAVE_RESP_ID  = 61;
    constexpr int RESERVED_ID_1       = 62;
    constexpr int RESERVED_ID_2       = 63;

    // Standard baud rates (bps)
    constexpr int BAUD_2400  = 2400;
    constexpr int BAUD_9600  = 9600;
    constexpr int BAUD_10417 = 10417;
    constexpr int BAUD_19200 = 19200;
    constexpr int BAUD_20000 = 20000;
}

// ── Frame types ───────────────────────────────────────────────────────────

enum class LINFrameType : uint8_t {
    Unconditional     = 0,  // Standard signal-carrying frame
    EventTriggered    = 1,  // Multiple slaves may respond
    Sporadic          = 2,  // Sent only when signals changed
    DiagnosticMaster  = 3,  // Master diagnostic request (ID 60)
    DiagnosticSlave   = 4,  // Slave diagnostic response (ID 61)
    Reserved          = 5,  // Reserved frame
    Unknown           = 6
};

// ── Checksum type ─────────────────────────────────────────────────────────

enum class LINChecksumType : uint8_t {
    Classic   = 0,  // LIN 1.x — over data field only
    Enhanced  = 1   // LIN 2.x — over PID + data field
};

// ── LIN bus state ─────────────────────────────────────────────────────────

enum class LINBusState : uint8_t {
    Idle         = 0,
    BreakField   = 1,
    SynchField   = 2,
    PIDField     = 3,
    DataField    = 4,
    ChecksumField = 5,
    Error        = 6
};

// ── Error flags ───────────────────────────────────────────────────────────

namespace LINError {
    constexpr uint8_t NONE            = 0x00;
    constexpr uint8_t BREAK_TOO_SHORT = 0x01;
    constexpr uint8_t SYNC_INVALID    = 0x02;
    constexpr uint8_t PARITY_ERROR    = 0x04;
    constexpr uint8_t CHECKSUM_ERROR  = 0x08;
    constexpr uint8_t NO_RESPONSE     = 0x10;
    constexpr uint8_t FRAMING_ERROR   = 0x20;
    constexpr uint8_t TIMEOUT         = 0x40;
}

// ═══════════════════════════════════════════════════════════════════════════
// Forward declarations
// ═══════════════════════════════════════════════════════════════════════════

struct LINSignal;
struct LDFNode;
struct LINScheduleEntry;

// ═══════════════════════════════════════════════════════════════════════════
// LINFrame — a single captured LIN frame
// ═══════════════════════════════════════════════════════════════════════════

struct LINFrame {
    int         bus        = 0;
    uint8_t     id         = 0;        // 6-bit LIN identifier (0-63)
    uint8_t     pid        = 0;        // 8-bit Protected ID (with parity)
    uint8_t     data[LIN::MAX_DATA_LEN] = {};
    uint8_t     dataLen    = 0;        // 1-8
    uint8_t     checksum   = 0;        // Received checksum
    uint8_t     calcChecksum = 0;      // Calculated checksum (for validation)
    LINChecksumType checksumType = LINChecksumType::Enhanced;
    LINFrameType frameType  = LINFrameType::Unconditional;
    LINBusState  state   = LINBusState::Idle;
    uint8_t     errorFlags = 0;
    bool        isReceived  = true;    // true = received, false = sent
    uint64_t    timestamp   = 0;       // microseconds
    int64_t     timedelta   = 0;       // delta from previous frame

    // ── Helpers ──────────────────────────────────────────────────────
    bool hasError() const { return errorFlags != LINError::NONE; }
    bool checksumValid() const { return checksum == calcChecksum; }

    QString idString() const {
        return QString("0x%1").arg(id, 2, 16, QChar('0')).toUpper();
    }

    QString dataHex() const {
        QString result;
        for (int i = 0; i < dataLen; ++i) {
            if (i > 0) result += " ";
            result += QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper();
        }
        return result;
    }

    QString dataASCII() const {
        QString result;
        for (int i = 0; i < dataLen; ++i) {
            if (data[i] >= 32 && data[i] <= 126)
                result += QChar(data[i]);
            else
                result += '.';
        }
        return result;
    }

    bool operator<(const LINFrame& other) const {
        return timestamp < other.timestamp;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// LINSignal — a signal defined in an LDF file
// ═══════════════════════════════════════════════════════════════════════════

struct LINSignal {
    QString     name;
    uint8_t     frameId     = 0;    // Which frame carries this signal
    uint8_t     startBit    = 0;    // Bit position within data field (0-63)
    uint8_t     length      = 1;    // Signal length in bits (1-16)
    double      factor      = 1.0;
    double      offset      = 0.0;
    double      minVal      = 0.0;
    double      maxVal      = 0.0;
    QString     unit;
    QString     publisher;         // Node that publishes this signal
    QVector<QString> subscribers;  // Nodes that consume this signal
    QMap<int32_t, QString> valueTable;  // Encoding values to text

    double decode(const uint8_t* data) const {
        if (!data) return 0.0;
        // Extract raw value from bit field (LSB first, LIN convention)
        uint32_t raw = 0;
        for (int b = 0; b < length; ++b) {
            int bitPos = startBit + b;
            int byteIdx = bitPos / 8;
            int bitInByte = bitPos % 8;
            if (byteIdx < LIN::MAX_DATA_LEN && (data[byteIdx] & (1 << bitInByte)))
                raw |= (1u << b);
        }
        return raw * factor + offset;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// LDFNode — a node (master or slave) defined in an LDF file
// ═══════════════════════════════════════════════════════════════════════════

struct LDFNode {
    QString     name;
    bool        isMaster    = false;
    uint8_t     NAD         = 0;    // Node Address for Diagnostic
    uint8_t     supplierId  = 0;
    uint8_t     functionId  = 0;
    uint8_t     variantId   = 0;
    double      protocolVer = 2.2;
    QVector<LINSignal> publishedSignals;
    QVector<LINSignal> subscribedSignals;
};

// ═══════════════════════════════════════════════════════════════════════════
// LINScheduleEntry — one entry in the LIN schedule table
// ═══════════════════════════════════════════════════════════════════════════

struct LINScheduleEntry {
    uint8_t     frameId     = 0;
    QString     frameName;
    uint32_t    delay_ms    = 0;    // Time slot duration in milliseconds
    LINFrameType type       = LINFrameType::Unconditional;
    bool        active      = true;
};

// ═══════════════════════════════════════════════════════════════════════════
// LDFDatabase — complete LDF file representation
// ═══════════════════════════════════════════════════════════════════════════

struct LDFDatabase {
    QString     protocolVersion;
    QString     languageVersion;
    uint32_t    baudRate        = 19200;
    QString     channelName;
    QVector<LDFNode> nodes;
    QVector<LINSignal> signalList;
    QVector<LINScheduleEntry> schedule;
    QMap<uint8_t, QString> frameNames;  // ID -> name mapping

    bool        isValid         = false;
    QString     errorString;

    LDFNode* masterNode() {
        for (auto& n : nodes)
            if (n.isMaster) return &n;
        return nullptr;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Parity and checksum helper functions
// ═══════════════════════════════════════════════════════════════════════════

namespace LINHelpers {

    /// Calculate the two parity bits (P0, P1) for a given 6-bit LIN ID.
    /// P0 = ID0 ^ ID1 ^ ID2 ^ ID4
    /// P1 = !(ID1 ^ ID3 ^ ID4 ^ ID5)
    inline uint8_t calcParity(uint8_t id6bit) {
        uint8_t p0 = ((id6bit >> 0) & 1) ^ ((id6bit >> 1) & 1)
                   ^ ((id6bit >> 2) & 1) ^ ((id6bit >> 4) & 1);
        uint8_t p1 = !(((id6bit >> 1) & 1) ^ ((id6bit >> 3) & 1)
                     ^ ((id6bit >> 4) & 1) ^ ((id6bit >> 5) & 1));
        return (p0 << 6) | (p1 << 7);
    }

    /// Build a Protected ID (PID) from a 6-bit ID
    inline uint8_t buildPID(uint8_t id6bit) {
        return (id6bit & LIN::PID_MASK_ID) | calcParity(id6bit);
    }

    /// Extract the 6-bit ID from a PID
    inline uint8_t pidToID(uint8_t pid) {
        return pid & LIN::PID_MASK_ID;
    }

    /// Verify parity bits in a PID
    inline bool verifyParity(uint8_t pid) {
        uint8_t id6 = pid & LIN::PID_MASK_ID;
        return (pid & LIN::PID_MASK_PARITY) == calcParity(id6);
    }

    /// Calculate classic checksum (LIN 1.x — over data only)
    inline uint8_t calcClassicChecksum(const uint8_t* data, int len) {
        uint16_t sum = 0;
        for (int i = 0; i < len; ++i)
            sum += data[i];
        // Add carry
        while (sum > 0xFF)
            sum = (sum & 0xFF) + (sum >> 8);
        return (uint8_t)(~sum);
    }

    /// Calculate enhanced checksum (LIN 2.x — over PID + data)
    inline uint8_t calcEnhancedChecksum(uint8_t pid, const uint8_t* data, int len) {
        uint16_t sum = pid;
        for (int i = 0; i < len; ++i)
            sum += data[i];
        while (sum > 0xFF)
            sum = (sum & 0xFF) + (sum >> 8);
        return (uint8_t)(~sum);
    }

    /// Determine checksum type by ID (per LIN 2.2A spec)
    inline LINChecksumType checksumTypeForID(uint8_t id) {
        // IDs 0-59 use classic; 60-63 use enhanced
        return (id >= 60) ? LINChecksumType::Enhanced : LINChecksumType::Classic; // Spec says enhanced for diag; classic for signals
        // NOTE: LIN 2.x actually uses enhanced for all IDs 0-63.
        // Classic is only for LIN 1.x. We default to enhanced and let user override.
    }

    /// Get frame type from ID
    inline LINFrameType frameTypeFromID(uint8_t id) {
        if (id == LIN::DIAG_MASTER_REQ_ID)
            return LINFrameType::DiagnosticMaster;
        if (id == LIN::DIAG_SLAVE_RESP_ID)
            return LINFrameType::DiagnosticSlave;
        if (id >= LIN::RESERVED_ID_1)
            return LINFrameType::Reserved;
        return LINFrameType::Unconditional;
    }

    /// Decode a complete LIN frame from raw byte array (as received from hardware)
    /// Format: [PID] [Data(0..7)] [Checksum]
    /// Returns true if frame seems valid
    inline bool decodeRawFrame(const QByteArray& raw, LINFrame& frame) {
        if (raw.size() < 3) // Minimum: PID + 1 data + checksum
            return false;

        frame.pid  = (uint8_t)raw[0];
        frame.id   = pidToID(frame.pid);

        int dataLen = raw.size() - 2; // subtract PID and checksum
        if (dataLen < 1 || dataLen > LIN::MAX_DATA_LEN)
            return false;

        frame.dataLen = dataLen;
        for (int i = 0; i < dataLen; ++i)
            frame.data[i] = (uint8_t)raw[1 + i];

        frame.checksum = (uint8_t)raw[raw.size() - 1];

        // Determine checksum type
        frame.checksumType = LINChecksumType::Enhanced;
        // frame.checksumType = checksumTypeForID(frame.id);

        // Calculate expected checksum
        if (frame.checksumType == LINChecksumType::Classic)
            frame.calcChecksum = calcClassicChecksum(frame.data, dataLen);
        else
            frame.calcChecksum = calcEnhancedChecksum(frame.pid, frame.data, dataLen);

        // Verify parity
        if (!verifyParity(frame.pid))
            frame.errorFlags |= LINError::PARITY_ERROR;

        if (!frame.checksumValid())
            frame.errorFlags |= LINError::CHECKSUM_ERROR;

        frame.frameType = frameTypeFromID(frame.id);

        return true;
    }

    /// Encode a LIN frame to raw byte array for transmission
    inline QByteArray encodeRawFrame(const LINFrame& frame) {
        QByteArray raw;
        raw.append((char)frame.pid);
        for (int i = 0; i < frame.dataLen; ++i)
            raw.append((char)frame.data[i]);
        // Recalculate checksum
        uint8_t cs;
        if (frame.checksumType == LINChecksumType::Classic)
            cs = calcClassicChecksum(frame.data, frame.dataLen);
        else
            cs = calcEnhancedChecksum(frame.pid, frame.data, frame.dataLen);
        raw.append((char)cs);
        return raw;
    }

} // namespace LINHelpers

#endif // LIN_STRUCTS_H
