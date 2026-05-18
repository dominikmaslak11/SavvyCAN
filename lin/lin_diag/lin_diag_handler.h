#ifndef LIN_DIAG_HANDLER_H
#define LIN_DIAG_HANDLER_H

#include <QObject>
#include <QVector>
#include "lin_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// LINDiagHandler — processes LIN diagnostic frames (IDs 60-61)
// Provides decoding of master requests and slave responses per LIN 2.2A
// ═══════════════════════════════════════════════════════════════════════════

// Diagnostic service identifiers (partial list from ISO 15765-2 / LIN 2.2A)
namespace LINDiag {
    constexpr uint8_t SID_READ_BY_IDENTIFIER  = 0xB2;
    constexpr uint8_t SID_READ_BY_ADDRESS     = 0x23;
    constexpr uint8_t SID_WRITE_BY_ADDRESS    = 0x3D;
    constexpr uint8_t SID_SESSION_CONTROL     = 0x10;
    constexpr uint8_t SID_TESTER_PRESENT      = 0x3E;
    constexpr uint8_t SID_ECU_RESET           = 0x11;
    constexpr uint8_t SID_READ_DTC_INFORMATION = 0x19;
    constexpr uint8_t SID_CLEAR_DTC           = 0x14;
    constexpr uint8_t SID_IO_CONTROL          = 0x2F;
    constexpr uint8_t SID_ROUTINE_CONTROL     = 0x31;
    constexpr uint8_t SID_REQ_DOWNLOAD        = 0x34;
    constexpr uint8_t SID_REQ_UPLOAD          = 0x35;
    constexpr uint8_t SID_TRANSFER_DATA       = 0x36;
    constexpr uint8_t SID_REQ_TRANSFER_EXIT   = 0x37;

    // Response modifiers
    constexpr uint8_t SID_POSITIVE_RESP_OFFSET = 0x40;
    constexpr uint8_t SID_NEGATIVE_RESPONSE    = 0x7F;

    // NRC (Negative Response Codes)
    constexpr uint8_t NRC_GENERAL_REJECT          = 0x10;
    constexpr uint8_t NRC_SERVICE_NOT_SUPPORTED   = 0x11;
    constexpr uint8_t NRC_SUBFUNC_NOT_SUPPORTED   = 0x12;
    constexpr uint8_t NRC_INCORRECT_MESSAGE_LENGTH = 0x13;
    constexpr uint8_t NRC_CONDITIONS_NOT_CORRECT  = 0x22;
    constexpr uint8_t NRC_REQUEST_OUT_OF_RANGE    = 0x31;
    constexpr uint8_t NRC_SECURITY_ACCESS_DENIED  = 0x33;
    constexpr uint8_t NRC_INVALID_KEY             = 0x35;
    constexpr uint8_t NRC_RESPONSE_PENDING        = 0x78;
}

struct LINDiagMessage {
    uint8_t     NAD = 0;        // Node Address for Diagnostic
    uint8_t     PCI = 0;        // Protocol Control Information
    uint8_t     SID = 0;        // Service Identifier
    uint8_t     subFunction = 0;
    QVector<uint8_t> data;      // Additional data bytes
    bool        isPositiveResponse = false;
    bool        isNegativeResponse = false;
    uint8_t     NRC = 0;        // Negative Response Code (if applicable)
};

class LINDiagHandler : public QObject
{
    Q_OBJECT

public:
    explicit LINDiagHandler(QObject *parent = nullptr);

    /// Decode a diagnostic master request frame (ID 60)
    static LINDiagMessage decodeMasterRequest(const LINFrame &frame);

    /// Decode a diagnostic slave response frame (ID 61)
    static LINDiagMessage decodeSlaveResponse(const LINFrame &frame);

    /// Check if a frame is a diagnostic frame
    static bool isDiagFrame(const LINFrame &frame);

    /// Get human-readable service name
    static QString serviceName(uint8_t sid);

    /// Get human-readable negative response code name
    static QString nrcName(uint8_t nrc);

    /// Get textual description of a diagnostic message
    static QString describeMessage(const LINDiagMessage &msg);

signals:
    void diagMessageDecoded(const LINDiagMessage &msg);
};

#endif // LIN_DIAG_HANDLER_H
