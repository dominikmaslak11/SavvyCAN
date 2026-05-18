#include "j1939_handler.h"
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════════════
// Built-in common J1939 PGN/SPN database
// ═══════════════════════════════════════════════════════════════════════════

static void populateBuiltinDB(J1939_DB &db) {
    db.name = "J1939-71 Built-in";
    db.isValid = true;

    // ── EEC1 — Electronic Engine Controller 1 (PGN 61444 / 0xF004) ──────
    {
        J1939_PGN pgn;
        pgn.pgn = 0xF004;
        pgn.name = "Electronic Engine Controller 1";
        pgn.acronym = "EEC1";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 20;

        J1939_SPN spn;
        spn.spn = 512; spn.name = "Engine Speed"; spn.pgn = 0xF004;
        spn.startByte = 3; spn.bitLength = 16; spn.resolution = 0.125;
        spn.unit = "rpm"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 513; spn.name = "Engine Speed At Idle Point 1";
        spn.startByte = 1; spn.bitLength = 8; spn.resolution = 0.125;
        spn.unit = "rpm"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 190; spn.name = "Engine Speed At Idle";
        spn.startByte = 0; spn.bitLength = 16; spn.resolution = 0.125;
        spn.unit = "rpm"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 544; spn.name = "Engine Reference Torque";
        spn.startByte = 5; spn.bitLength = 16; spn.resolution = 1.0;
        spn.unit = "Nm"; spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // ── EEC2 — Electronic Engine Controller 2 (PGN 61443 / 0xF003) ──────
    {
        J1939_PGN pgn;
        pgn.pgn = 0xF003;
        pgn.name = "Electronic Engine Controller 2";
        pgn.acronym = "EEC2";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 50;

        J1939_SPN spn;
        spn.spn = 91; spn.name = "Accelerator Pedal Position 1";
        spn.startByte = 1; spn.bitLength = 8; spn.resolution = 0.4;
        spn.unit = "%"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 92; spn.name = "Engine Percent Load At Current Speed";
        spn.startByte = 2; spn.bitLength = 8; spn.resolution = 1.0;
        spn.unit = "%"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 94; spn.name = "Engine Fuel Delivery Pressure";
        spn.startByte = 3; spn.bitLength = 8; spn.resolution = 4.0;
        spn.unit = "kPa"; spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // ── ET1 — Engine Temperature 1 (PGN 65262 / 0xFEEE) ──────────────────
    {
        J1939_PGN pgn;
        pgn.pgn = 0xFEEE;
        pgn.name = "Engine Temperature 1";
        pgn.acronym = "ET1";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 1000;

        J1939_SPN spn;
        spn.spn = 110; spn.name = "Engine Coolant Temperature";
        spn.startByte = 0; spn.bitLength = 8; spn.resolution = 1.0; spn.offset = -40.0;
        spn.unit = "\u00b0C"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 174; spn.name = "Engine Fuel Temperature 1";
        spn.startByte = 1; spn.bitLength = 8; spn.resolution = 1.0; spn.offset = -40.0;
        spn.unit = "\u00b0C"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 175; spn.name = "Engine Oil Temperature 1";
        spn.startByte = 2; spn.bitLength = 16; spn.resolution = 0.03125; spn.offset = -273.0;
        spn.unit = "\u00b0C"; spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // ── CCVS1 — Cruise Control / Vehicle Speed 1 (PGN 65265 / 0xFEF1) ───
    {
        J1939_PGN pgn;
        pgn.pgn = 0xFEF1;
        pgn.name = "Cruise Control / Vehicle Speed 1";
        pgn.acronym = "CCVS1";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 100;

        J1939_SPN spn;
        spn.spn = 84; spn.name = "Wheel-Based Vehicle Speed";
        spn.startByte = 0; spn.bitLength = 16; spn.resolution = 0.00390625;
        spn.unit = "km/h"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 86; spn.name = "Cruise Control Set Speed";
        spn.startByte = 2; spn.bitLength = 8; spn.resolution = 1.0;
        spn.unit = "km/h"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 597; spn.name = "Brake Switch";
        spn.startByte = 4; spn.startBit = 4; spn.bitLength = 2;
        spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // ── LFE — Engine Fuel Economy (PGN 65266 / 0xFEF2) ──────────────────
    {
        J1939_PGN pgn;
        pgn.pgn = 0xFEF2;
        pgn.name = "Engine Fuel Economy";
        pgn.acronym = "LFE";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 100;

        J1939_SPN spn;
        spn.spn = 183; spn.name = "Engine Fuel Rate";
        spn.startByte = 0; spn.bitLength = 16; spn.resolution = 0.05;
        spn.unit = "L/h"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 184; spn.name = "Engine Instantaneous Fuel Economy";
        spn.startByte = 2; spn.bitLength = 16; spn.resolution = 0.001953125;
        spn.unit = "km/L"; spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // ── AMB — Ambient Conditions (PGN 65269 / 0xFEF5) ───────────────────
    {
        J1939_PGN pgn;
        pgn.pgn = 0xFEF5;
        pgn.name = "Ambient Conditions";
        pgn.acronym = "AMB";
        pgn.dataLength = 8;
        pgn.updateRate_ms = 1000;

        J1939_SPN spn;
        spn.spn = 108; spn.name = "Barometric Pressure";
        spn.startByte = 0; spn.bitLength = 8; spn.resolution = 0.5;
        spn.unit = "kPa"; spn.dataLength = 8;
        pgn.spns.append(spn);

        spn = J1939_SPN();
        spn.spn = 171; spn.name = "Ambient Air Temperature";
        spn.startByte = 4; spn.bitLength = 16; spn.resolution = 0.03125; spn.offset = -273.0;
        spn.unit = "\u00b0C"; spn.dataLength = 8;
        pgn.spns.append(spn);

        db.pgns[pgn.pgn] = pgn;
    }

    // Register SPNs in the global SPN map
    for (auto &pgnEntry : db.pgns) {
        for (const auto &spn : pgnEntry.spns) {
            db.spns[spn.spn] = spn;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// J1939Handler implementation
// ═══════════════════════════════════════════════════════════════════════════

J1939Handler::J1939Handler(QObject *parent)
    : QObject(parent)
{
    populateBuiltinDB(mDB);
}

void J1939Handler::setDatabase(const J1939_DB &db)
{
    mDB = db;
}

QMap<uint32_t, double> J1939Handler::decodeSPNs(uint32_t pgn, const uint8_t *data, int len) const
{
    QMap<uint32_t, double> result;
    const J1939_PGN *pgdef = mDB.findPGN(pgn);
    if (!pgdef) return result;

    for (const auto &spn : pgdef->spns) {
        if (spn.startByte + (spn.bitLength + 7) / 8 <= len) {
            double val = spn.decode(data, len);
            result[spn.spn] = val;
        }
    }
    return result;
}

bool J1939Handler::processFrame(const CANFrame &frame)
{
    if (!frame.hasExtendedFrameFormat()) return false;

    J1939_ID jid = J1939_ID::fromCANID(frame.frameId());
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    int len = frame.payload().size();

    // ── Transport Protocol ───────────────────────────────────────────────
    if (jid.pgn == J1939Const::PGN_TP_CM) {
        handleTransportCM(frame);
        return false; // TP.CM alone doesn't produce decoded output
    }
    if (jid.pgn == J1939Const::PGN_TP_DT) {
        handleTransportDT(frame);
        return false;
    }

    // ── Address Claiming ─────────────────────────────────────────────────
    if (jid.pgn == J1939Const::PGN_ADDR_CLAIMED && len >= 8) {
        handleAddressClaim(frame);
    }

    // ── Diagnostics ──────────────────────────────────────────────────────
    if (jid.pgn == J1939Const::PGN_DM1_ACTIVE_DTC) {
        handleDM1(frame);
    }
    if (jid.pgn == J1939Const::PGN_DM2_PREV_DTC) {
        handleDM2(frame);
    }

    // ── Standard PGN decode ──────────────────────────────────────────────
    mLastPGN = jid.pgn;

    // For single-frame messages (≤8 bytes): decode SPNs directly
    if (len >= 1 && len <= 8) {
        QMap<uint32_t, double> values = decodeSPNs(jid.pgn, data, len);
        if (!values.isEmpty()) {
            emit pgnDecoded(jid.pgn, values);
            return true;
        }
    }

    return false;
}

// ── Transport Protocol: Connection Management ───────────────────────────

void J1939Handler::handleTransportCM(const CANFrame &frame)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    if (frame.payload().size() < 8) return;

    uint8_t controlByte = data[0];  // RTS=16, CTS=17, EOM=19, BAM=32, Abort=255
    uint16_t totalBytes = data[1] | (data[2] << 8);
    uint8_t totalPackets = data[3];
    uint32_t pgn = data[5] | (data[6] << 8) | (data[7] << 16);

    J1939_ID jid = J1939_ID::fromCANID(frame.frameId());

    if (controlByte == 16) { // RTS — Ready To Send
        QPair<uint8_t, uint8_t> key(jid.sa, pgn & 0xFF);
        TPState &state = mTPStates[key];
        state.active = true;
        state.pgn = pgn;
        state.srcAddr = jid.sa;
        state.totalPackets = totalPackets;
        state.totalBytes = totalBytes;
        state.nextPacket = 0;
        state.buffer.clear();
        state.buffer.reserve(totalBytes);
        state.timer.start();
    }
    else if (controlByte == 32) { // BAM — Broadcast Announce Message
        QPair<uint8_t, uint8_t> key(jid.sa, pgn & 0xFF);
        TPState &state = mTPStates[key];
        state.active = true;
        state.pgn = pgn;
        state.srcAddr = jid.sa;
        state.totalPackets = totalPackets;
        state.totalBytes = totalBytes;
        state.nextPacket = 0;
        state.buffer.clear();
        state.buffer.reserve(totalBytes);
        state.timer.start();
    }
    else if (controlByte == 255 || controlByte == 19) { // Abort or EOM
        QPair<uint8_t, uint8_t> key(jid.sa, pgn & 0xFF);
        if (mTPStates.contains(key)) {
            TPState &st = mTPStates[key];
            if (controlByte == 19 && st.buffer.size() > 0) {
                // End of Message — deliver assembled data
                emit transportComplete(st.pgn, st.buffer);
            }
            mTPStates.remove(key);
        }
    }
}

// ── Transport Protocol: Data Transfer ────────────────────────────────────

void J1939Handler::handleTransportDT(const CANFrame &frame)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    if (frame.payload().size() < 8) return;

    uint8_t sequenceNum = data[0]; // 1-255 sequential
    J1939_ID jid = J1939_ID::fromCANID(frame.frameId());

    for (auto it = mTPStates.begin(); it != mTPStates.end(); ++it) {
        TPState &state = it.value();
        if (!state.active) continue;
        if (state.srcAddr != jid.sa) continue;

        if (sequenceNum == state.nextPacket + 1) {
            state.buffer.append(reinterpret_cast<const char*>(data + 1), 7);
            state.nextPacket = sequenceNum;

            if (state.nextPacket >= state.totalPackets || state.buffer.size() >= (int)state.totalBytes) {
                // Complete
                emit transportComplete(state.pgn, state.buffer);
                state.active = false;
            }
            break;
        }
    }
}

// ── DM1: Active Diagnostic Trouble Codes ────────────────────────────────

void J1939Handler::handleDM1(const CANFrame &frame)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    int len = frame.payload().size();
    if (len < 2) return;

    uint8_t milStatus = (data[0] >> 6) & 0x03;
    uint8_t rslStatus = (data[0] >> 4) & 0x03;
    uint8_t awlStatus = (data[0] >> 2) & 0x03;
    uint8_t plStatus = data[0] & 0x03;

    Q_UNUSED(milStatus); Q_UNUSED(rslStatus); Q_UNUSED(awlStatus); Q_UNUSED(plStatus);
    mActiveDTCs.clear();

    for (int i = 1; i + 3 < len; i += 4) {
        J1939_DTC dtc = J1939_DTC::fromBytes(data + i);
        mActiveDTCs.append(dtc);
    }

    emit dtcUpdated(mActiveDTCs);
}

void J1939Handler::handleDM2(const CANFrame &frame)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    int len = frame.payload().size();
    if (len < 2) return;

    mPreviousDTCs.clear();
    for (int i = 1; i + 3 < len; i += 4) {
        J1939_DTC dtc = J1939_DTC::fromBytes(data + i);
        mPreviousDTCs.append(dtc);
    }
}

// ── Address Claiming ────────────────────────────────────────────────────

void J1939Handler::handleAddressClaim(const CANFrame &frame)
{
    const uint8_t *data = reinterpret_cast<const uint8_t*>(frame.payload().constData());
    J1939_ID jid = J1939_ID::fromCANID(frame.frameId());

    J1939_AddressClaim claim;
    claim.sourceAddr = jid.sa;
    claim.deviceName = data[0] | (data[1] << 8) | (data[2] << 16) | ((uint32_t)data[3] << 24);
    claim.claimed = true;
    claim.claimTime = frame.timeStamp().microSeconds();

    mAddressClaims[jid.sa] = claim;
    emit addressClaimed(claim);
}
