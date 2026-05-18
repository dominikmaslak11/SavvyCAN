#include "lin_diag_handler.h"

LINDiagHandler::LINDiagHandler(QObject *parent)
    : QObject(parent)
{
}

bool LINDiagHandler::isDiagFrame(const LINFrame &frame)
{
    return (frame.id == LIN::DIAG_MASTER_REQ_ID ||
            frame.id == LIN::DIAG_SLAVE_RESP_ID);
}

LINDiagMessage LINDiagHandler::decodeMasterRequest(const LINFrame &frame)
{
    LINDiagMessage msg;

    if (frame.id != LIN::DIAG_MASTER_REQ_ID || frame.dataLen < 3)
        return msg;

    // Master Request frame format:
    // Byte 0: NAD (Node Address for Diagnostic)
    // Byte 1: PCI (Protocol Control Information) — first nibble = type
    // Byte 2: SID (Service Identifier)
    // Byte 3+: Data
    msg.NAD = frame.data[0];
    msg.PCI = frame.data[1];
    msg.SID = frame.data[2];

    if (frame.dataLen >= 4)
        msg.subFunction = frame.data[3];

    for (int i = 4; i < frame.dataLen; ++i)
        msg.data.append(frame.data[i]);

    return msg;
}

LINDiagMessage LINDiagHandler::decodeSlaveResponse(const LINFrame &frame)
{
    LINDiagMessage msg;

    if (frame.id != LIN::DIAG_SLAVE_RESP_ID || frame.dataLen < 3)
        return msg;

    // Slave Response frame format:
    // Byte 0: NAD
    // Byte 1: PCI
    // Byte 2: SID (or 0x7F for negative response)
    msg.NAD = frame.data[0];
    msg.PCI = frame.data[1];
    msg.SID = frame.data[2];

    // Check for negative response
    if (msg.SID == LINDiag::SID_NEGATIVE_RESPONSE) {
        msg.isNegativeResponse = true;
        if (frame.dataLen >= 4) {
            msg.SID = frame.data[3]; // The SID that was rejected
            if (frame.dataLen >= 5)
                msg.NRC = frame.data[4];
        }
        return msg;
    }

    // Check for positive response
    if (msg.SID >= 0x40 && msg.SID <= 0x7E) {
        msg.isPositiveResponse = true;
        // The actual service SID is msg.SID - 0x40
    }

    if (frame.dataLen >= 4)
        msg.subFunction = frame.data[3];

    for (int i = 4; i < frame.dataLen; ++i)
        msg.data.append(frame.data[i]);

    return msg;
}

QString LINDiagHandler::serviceName(uint8_t sid)
{
    switch (sid) {
    case LINDiag::SID_READ_BY_IDENTIFIER:  return "Read Data By Identifier (0xB2)";
    case LINDiag::SID_READ_BY_ADDRESS:     return "Read Memory By Address (0x23)";
    case LINDiag::SID_WRITE_BY_ADDRESS:    return "Write Memory By Address (0x3D)";
    case LINDiag::SID_SESSION_CONTROL:     return "Diagnostic Session Control (0x10)";
    case LINDiag::SID_TESTER_PRESENT:      return "Tester Present (0x3E)";
    case LINDiag::SID_ECU_RESET:           return "ECU Reset (0x11)";
    case LINDiag::SID_READ_DTC_INFORMATION: return "Read DTC Information (0x19)";
    case LINDiag::SID_CLEAR_DTC:           return "Clear Diagnostic Information (0x14)";
    case LINDiag::SID_IO_CONTROL:          return "InputOutput Control By Identifier (0x2F)";
    case LINDiag::SID_ROUTINE_CONTROL:     return "Routine Control (0x31)";
    case LINDiag::SID_REQ_DOWNLOAD:        return "Request Download (0x34)";
    case LINDiag::SID_REQ_UPLOAD:          return "Request Upload (0x35)";
    case LINDiag::SID_TRANSFER_DATA:       return "Transfer Data (0x36)";
    case LINDiag::SID_REQ_TRANSFER_EXIT:   return "Request Transfer Exit (0x37)";
    default:                               return QString("Unknown Service (0x%1)").arg(sid, 2, 16, QChar('0'));
    }
}

QString LINDiagHandler::nrcName(uint8_t nrc)
{
    switch (nrc) {
    case LINDiag::NRC_GENERAL_REJECT:           return "General Reject (0x10)";
    case LINDiag::NRC_SERVICE_NOT_SUPPORTED:    return "Service Not Supported (0x11)";
    case LINDiag::NRC_SUBFUNC_NOT_SUPPORTED:    return "SubFunction Not Supported (0x12)";
    case LINDiag::NRC_INCORRECT_MESSAGE_LENGTH: return "Incorrect Message Length (0x13)";
    case LINDiag::NRC_CONDITIONS_NOT_CORRECT:   return "Conditions Not Correct (0x22)";
    case LINDiag::NRC_REQUEST_OUT_OF_RANGE:     return "Request Out Of Range (0x31)";
    case LINDiag::NRC_SECURITY_ACCESS_DENIED:   return "Security Access Denied (0x33)";
    case LINDiag::NRC_INVALID_KEY:              return "Invalid Key (0x35)";
    case LINDiag::NRC_RESPONSE_PENDING:         return "Response Pending (0x78)";
    default:                                    return QString("Unknown NRC (0x%1)").arg(nrc, 2, 16, QChar('0'));
    }
}

QString LINDiagHandler::describeMessage(const LINDiagMessage &msg)
{
    QString desc;

    desc += QString("NAD: 0x%1  ").arg(msg.NAD, 2, 16, QChar('0'));

    if (msg.isNegativeResponse) {
        desc += "NEGATIVE RESPONSE: ";
        desc += serviceName(msg.SID);
        desc += QString("  <- NRC: %1").arg(nrcName(msg.NRC));
    } else if (msg.isPositiveResponse) {
        desc += "POSITIVE RESPONSE: ";
        desc += serviceName(msg.SID - 0x40);
    } else {
        desc += "REQUEST: ";
        desc += serviceName(msg.SID);
    }

    if (msg.subFunction) {
        desc += QString("  SubFn: 0x%1").arg(msg.subFunction, 2, 16, QChar('0'));
    }

    if (!msg.data.isEmpty()) {
        desc += "  Data: ";
        for (uint8_t b : msg.data)
            desc += QString("0x%1 ").arg(b, 2, 16, QChar('0'));
    }

    return desc;
}
