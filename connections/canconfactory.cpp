#include <QString>
#include "canconfactory.h"
#include "serialbusconnection.h"
#include "gvretserial.h"
#include "mqtt_bus.h"
#include "socketcand.h"
#include "lawicel_serial.h"
#include "canserver.h"
#include "canlogserver.h"
#include "lin/lin_connection.h"
#include "lin/lin_can_bridge.h"

using namespace CANCon;

CANConnection* CanConFactory::create(type pType, QString pPortName, QString pDriverName, int pSerialSpeed, int pBusSpeed, bool pCanFd, int pDataRate)
{
    switch(pType) {
    case SERIALBUS:
      return new SerialBusConnection(pPortName, pDriverName, pBusSpeed, pDataRate, pCanFd);
    case GVRET_SERIAL:
        if(pPortName.contains(".") && !pPortName.contains("tty") && !pPortName.contains("serial"))
        return new GVRetSerial(pPortName, true);
        else
        return new GVRetSerial(pPortName, false);
    case REMOTE:
        return new GVRetSerial(pPortName, true);  //it's a special case of GVRET connected over TCP/IP so it uses the same class
    case LAWICEL:
        return new LAWICELSerial(pPortName, pSerialSpeed, pBusSpeed, pCanFd, pDataRate);
    case KAYAK:
        return new SocketCANd(pPortName);
    case MQTT:
        return new MQTT_BUS(pPortName);
    case CANSERVER:
        return new CANserver(pPortName);
    case CANLOGSERVER:
        return new CanLogServer(pPortName);
    case LIN_SERIAL: {
        auto *linConn = new LINSerialConnection(pPortName, pBusSpeed);
        auto *bridge = new LinCanBridge(linConn, pPortName, pBusSpeed, 1);
        return bridge;
    }
    case LIN_SOCKETCAN: {
#ifdef Q_OS_LINUX
        auto *linConn = new LINSocketCANConnection(pPortName);
        auto *bridge = new LinCanBridge(linConn, pPortName, LIN::BAUD_19200, 1);
        return bridge;
#else
        Q_UNUSED(pBusSpeed)
        return nullptr;
#endif
    }
    default: {}
    }

    return nullptr;
}
