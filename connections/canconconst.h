#ifndef CANCONCONST_H
#define CANCONCONST_H

namespace CANCon {

    /**
     * @brief The status enum
     */
    enum status
    {
        NOT_CONNECTED,  /*!< device is not connected */
        CONNECTED       /*!< device is connected */
    };

    enum type
    {
        GVRET_SERIAL,
        KVASER,
        SERIALBUS,
        REMOTE,
        KAYAK,
        MQTT,
        LAWICEL,
        CANSERVER,
        CANLOGSERVER,
        LIN_SERIAL,
        LIN_SOCKETCAN,
        NONE
    };

    // ── Standard bus speeds (bps) ─────────────────────────────────────
    namespace Speed {
        constexpr int CAN_10K    = 10000;
        constexpr int CAN_20K    = 20000;
        constexpr int CAN_50K    = 50000;
        constexpr int CAN_83K    = 83333;
        constexpr int CAN_100K   = 100000;
        constexpr int CAN_125K   = 125000;
        constexpr int CAN_250K   = 250000;
        constexpr int CAN_500K   = 500000;
        constexpr int CAN_800K   = 800000;
        constexpr int CAN_1M     = 1000000;
        constexpr int CAN_FD_1M  = 1000000;
        constexpr int CAN_FD_2M  = 2000000;
        constexpr int CAN_FD_4M  = 4000000;
        constexpr int CAN_FD_5M  = 5000000;
    }

    // ── Standard serial speeds (bps) ───────────────────────────────────
    namespace SerialSpeed {
        constexpr int BAUD_9600    = 9600;
        constexpr int BAUD_19200   = 19200;
        constexpr int BAUD_38400   = 38400;
        constexpr int BAUD_57600   = 57600;
        constexpr int BAUD_115200  = 115200;
        constexpr int BAUD_150000  = 150000;
        constexpr int BAUD_250000  = 250000;
        constexpr int BAUD_500000  = 500000;
        constexpr int BAUD_1M      = 1000000;
        constexpr int BAUD_2M      = 2000000;
        constexpr int BAUD_3M      = 3000000;
    }

    // ── Default network ports ──────────────────────────────────────────
    namespace Port {
        constexpr int GVRET_UDP      = 17222;
        constexpr int GVRET_TCP      = 23;
        constexpr int KAYAK_UDP      = 42000;
        constexpr int CAN_SERVER_UDP = 1338;
    }
}

class CANConStatus
{
public:
    CANCon::status conStatus;
    int numHardwareBuses;
};

#endif // CANCONCONST_H
