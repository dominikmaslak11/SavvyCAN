#ifndef J1939_STRUCTS_H
#define J1939_STRUCTS_H

#include <QString>
#include <QVector>
#include <QMap>
#include <stdint.h>

// ═══════════════════════════════════════════════════════════════════════════
// J1939 data structures for SavvyCAN
// Based on SAE J1939-71 (Vehicle Application Layer) and J1939-73 (Diagnostics)
// ═══════════════════════════════════════════════════════════════════════════

namespace J1939Const {
    constexpr uint32_t MASK_PRIORITY   = 0x1C000000;  // bits 26-28
    constexpr uint32_t MASK_EDP        = 0x02000000;  // bit 25
    constexpr uint32_t MASK_DP         = 0x01000000;  // bit 24
    constexpr uint32_t MASK_PF         = 0x00FF0000;  // bits 16-23
    constexpr uint32_t MASK_PS         = 0x0000FF00;  // bits 8-15
    constexpr uint32_t MASK_SA         = 0x000000FF;  // bits 0-7
    constexpr uint32_t MASK_PGN        = 0x03FFFF00;  // PGN = R+DP+PF+PS

    constexpr int GLOBAL_DEST = 0xFF;   // broadcast destination address
    constexpr int NULL_ADDR   = 0xFE;   // null address

    // Transport Protocol PGNs
    constexpr uint32_t PGN_TP_CM  = 0xEC00;  // Connection Management (RTS/CTS)
    constexpr uint32_t PGN_TP_DT  = 0xEB00;  // Data Transfer
    constexpr uint32_t PGN_TP_BAM = 0xEC00;  // BAM (same PGN as TP.CM)

    // Diagnostic PGNs
    constexpr uint32_t PGN_DM1_ACTIVE_DTC    = 0xFECA;
    constexpr uint32_t PGN_DM2_PREV_DTC      = 0xFECB;
    constexpr uint32_t PGN_DM3_CLEAR_DTC     = 0xFECC;
    constexpr uint32_t PGN_DM11_DTC_CLEAR_REQ = 0xFED3;
    constexpr uint32_t PGN_DM14_MEM_ACCESS    = 0xFED6;
    constexpr uint32_t PGN_DM15_MEM_RESPONSE  = 0xFED7;
    constexpr uint32_t PGN_DM16_BINARY_DATA   = 0xFED8;

    // Address Claiming PGNs
    constexpr uint32_t PGN_ADDR_CLAIMED     = 0xEE00;
    constexpr uint32_t PGN_CMD_ADDR         = 0xFED8;
    constexpr uint32_t PGN_REQUEST          = 0xEA00;

    // Common PGNs
    constexpr uint32_t PGN_EC1_ENGINE_CONFIG = 0xFEE3;
    constexpr uint32_t PGN_EC2_ENGINE_FLUID  = 0xFE48;
    constexpr uint32_t PGN_EEC1              = 0xF004;
    constexpr uint32_t PGN_EEC2              = 0xF003;
    constexpr uint32_t PGN_ET1               = 0xFEEE;
    constexpr uint32_t PGN_LFE               = 0xFEF2;
    constexpr uint32_t PGN_LFI               = 0xFEEE;
    constexpr uint32_t PGN_AMB_COND          = 0xFEF5;
    constexpr uint32_t PGN_CCVS1             = 0xFEF1;
}

// ── Decoded J1939 Identifier ──────────────────────────────────────────────

struct J1939_ID {
    uint8_t  priority   = 6;    // 3-bit priority (0=highest, 7=lowest)
    uint8_t  edp        = 0;    // Extended Data Page
    uint8_t  dp         = 0;    // Data Page
    uint8_t  pf         = 0;    // PDU Format
    uint8_t  ps         = 0;    // PDU Specific
    uint8_t  sa         = 0;    // Source Address
    uint32_t pgn        = 0;    // Parameter Group Number (18-bit)
    uint8_t  da         = 0xFF; // Destination Address (PDU2 format)
    bool     isPDU1     = true; // PDU1: PF 0-239 uses DA; PDU2: PF 240-255 uses GE
    bool     isBroadcast = true;

    /// Decode from raw 29-bit CAN ID
    static J1939_ID fromCANID(uint32_t canId) {
        J1939_ID id;
        id.priority = (canId & J1939Const::MASK_PRIORITY) >> 26;
        id.edp      = (canId & J1939Const::MASK_EDP)      >> 25;
        id.dp       = (canId & J1939Const::MASK_DP)       >> 24;
        id.pf       = (canId & J1939Const::MASK_PF)       >> 16;
        id.ps       = (canId & J1939Const::MASK_PS)       >> 8;
        id.sa       = (canId & J1939Const::MASK_SA);
        id.pgn      = (canId & J1939Const::MASK_PGN)      >> 8;

        id.isPDU1 = (id.pf < 240);
        if (id.isPDU1) {
            id.da = id.ps;
            id.isBroadcast = (id.da == J1939Const::GLOBAL_DEST);
        } else {
            id.da = J1939Const::GLOBAL_DEST; // PDU2 is always broadcast
            id.isBroadcast = true;
        }

        return id;
    }

    /// Encode to a 29-bit CAN ID
    uint32_t toCANID() const {
        uint32_t id = 0;
        id |= (uint32_t(priority) << 26) & J1939Const::MASK_PRIORITY;
        id |= (uint32_t(edp)      << 25) & J1939Const::MASK_EDP;
        id |= (uint32_t(dp)       << 24) & J1939Const::MASK_DP;
        id |= (uint32_t(pf)       << 16) & J1939Const::MASK_PF;
        id |= (uint32_t(ps)       << 8)  & J1939Const::MASK_PS;
        id |= uint32_t(sa)               & J1939Const::MASK_SA;
        return id;
    }

    QString toString() const {
        return QString("PGN:%1 SA:0x%2 DA:0x%3 Prio:%4")
            .arg(pgn, 5, 16, QChar('0'))
            .arg(sa, 2, 16, QChar('0'))
            .arg(da, 2, 16, QChar('0'))
            .arg(priority);
    }
};

// ── SPN — Suspect Parameter Number ────────────────────────────────────────

struct J1939_SPN {
    uint32_t    spn        = 0;
    QString     name;
    uint16_t    pgn        = 0;       // PGN this SPN belongs to
    uint8_t     startByte  = 0;       // Starting byte in the PGN data
    uint8_t     startBit   = 0;       // Starting bit within the startByte
    uint16_t    bitLength  = 1;       // Length in bits
    double      resolution = 1.0;     // Scale factor
    double      offset     = 0.0;     // Additive offset
    double      minVal     = 0.0;
    double      maxVal     = 0.0;
    QString     unit;
    QString     description;
    bool        isSigned   = false;
    uint8_t     dataLength = 8;       // Expected total data length of the PGN

    /// Decode raw bytes into an engineering value
    double decode(const uint8_t* data, int len) const {
        if (!data || len < startByte + 1) return 0.0;

        // Extract raw value from bit field (Intel byte order / little-endian)
        uint64_t raw = 0;
        int totalBits = startByte * 8 + startBit + bitLength;
        if (totalBits / 8 >= len) return 0.0;

        for (int b = 0; b < bitLength; ++b) {
            int bitPos = startByte * 8 + startBit + b;
            int byteIdx = bitPos / 8;
            int bitInByte = bitPos % 8;
            if (data[byteIdx] & (1 << bitInByte))
                raw |= (1ULL << b);
        }

        // Handle signed values
        if (isSigned && (raw & (1ULL << (bitLength - 1)))) {
            // Sign-extend
            uint64_t signMask = ~((1ULL << bitLength) - 1);
            raw |= signMask;
            int64_t sval = static_cast<int64_t>(raw);
            return sval * resolution + offset;
        }

        return raw * resolution + offset;
    }

    /// Encode an engineering value into raw bytes
    void encode(double value, uint8_t* data, int len) const {
        if (!data || len < startByte + 1) return;

        uint64_t raw = static_cast<uint64_t>((value - offset) / resolution);
        for (int b = 0; b < bitLength; ++b) {
            int bitPos = startByte * 8 + startBit + b;
            int byteIdx = bitPos / 8;
            int bitInByte = bitPos % 8;
            if (raw & (1ULL << b))
                data[byteIdx] |= (1 << bitInByte);
            else
                data[byteIdx] &= ~(1 << bitInByte);
        }
    }
};

// ── PGN — Parameter Group Number definition ───────────────────────────────

struct J1939_PGN {
    uint32_t    pgn         = 0;
    QString     name;
    QString     acronym;
    uint8_t     dataLength  = 8;       // Expected data length
    uint16_t    updateRate_ms = 0;     // Typical update rate (0 = on request)
    QString     description;
    QVector<J1939_SPN> spns;           // SPNs in this PGN
    uint8_t     defaultPriority = 6;
    uint32_t    defaultRate_ms = 1000; // Default transmit rate

    /// Find an SPN within this PGN
    const J1939_SPN* findSPN(uint32_t spn) const {
        for (const auto &s : spns)
            if (s.spn == spn) return &s;
        return nullptr;
    }
};

// ── DTC — Diagnostic Trouble Code ─────────────────────────────────────────

struct J1939_DTC {
    uint32_t    spn         = 0;       // SPN of the fault
    uint8_t     fmi         = 0;       // Failure Mode Identifier (0-31)
    uint8_t     occurrence  = 0;       // Occurrence count
    uint8_t     conversion  = 0;       // SPN conversion method
    uint8_t     lampStatus  = 0;       // MIL/RSL/AWL/PL status bits
    QString     description;

    static J1939_DTC fromBytes(const uint8_t* data) {
        J1939_DTC dtc;
        dtc.spn = data[0] | (data[1] << 8) | ((data[2] & 0xE0) << 11);
        dtc.fmi = data[2] & 0x1F;
        dtc.occurrence = data[3] & 0x7F;
        dtc.conversion = (data[0] >> 2) & 0x01;
        dtc.lampStatus = (data[3] >> 7) | ((data[4] & 0x03) << 1);
        return dtc;
    }

    QString fmiDescription() const {
        switch (fmi) {
            case 0:  return "Data valid but above normal";
            case 1:  return "Data valid but below normal";
            case 2:  return "Data erratic/intermittent";
            case 3:  return "Voltage above normal / shorted high";
            case 4:  return "Voltage below normal / shorted low";
            case 5:  return "Current below normal / open circuit";
            case 6:  return "Current above normal / grounded circuit";
            case 7:  return "Mechanical system not responding";
            case 8:  return "Abnormal frequency/pulse width";
            case 9:  return "Abnormal update rate";
            case 10: return "Abnormal rate of change";
            case 11: return "Root cause unknown";
            case 12: return "Bad intelligent device";
            case 13: return "Out of calibration";
            case 14: return "Special instructions";
            case 15: return "Data valid but above normal (least severe)";
            case 16: return "Data valid but above normal (moderate severe)";
            case 17: return "Data valid but below normal (least severe)";
            case 18: return "Data valid but below normal (moderate severe)";
            case 19: return "Network error";
            case 31: return "Not available / condition exists";
            default: return QString("FMI %1").arg(fmi);
        }
    }

    QString toString() const {
        return QString("SPN:%1 FMI:%2 Count:%3 Lamps:0x%4")
            .arg(spn).arg(fmi).arg(occurrence)
            .arg(lampStatus, 2, 16, QChar('0'));
    }
};

// ── Address Claim ─────────────────────────────────────────────────────────

struct J1939_AddressClaim {
    uint8_t     sourceAddr  = 0;
    uint32_t    deviceName  = 0;       // 64-bit NAME (simplified to 32-bit here)
    QString     deviceType;            // e.g., "Engine #1", "Brake Controller"
    bool        claimed     = false;
    uint64_t    claimTime   = 0;       // microseconds when claimed
};

// ── J1939 PGN Database ────────────────────────────────────────────────────

struct J1939_DB {
    QString     name;
    QString     description;
    QMap<uint32_t, J1939_PGN> pgns;    // keyed by PGN number
    QMap<uint32_t, J1939_SPN> spns;    // keyed by SPN number
    bool        isValid     = false;

    const J1939_PGN* findPGN(uint32_t pgn) const {
        auto it = pgns.find(pgn);
        return (it != pgns.end()) ? &it.value() : nullptr;
    }

    const J1939_SPN* findSPN(uint32_t spn) const {
        auto it = spns.find(spn);
        return (it != spns.end()) ? &it.value() : nullptr;
    }
};

#endif // J1939_STRUCTS_H
