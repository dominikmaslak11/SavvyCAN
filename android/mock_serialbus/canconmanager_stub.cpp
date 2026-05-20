#include <QObject>
#include <QList>
#include "can_structs.h"
#include "canconnection.h"

// Minimal stub for CANConManager — provides the symbols that bus_protocols need.
// The real Android connection management is done through BluetoothManager/WifiManager/UsbHostManager.

class CANConManager : public QObject
{
    Q_OBJECT
public:
    static CANConManager *getInstance() {
        static CANConManager instance;
        return &instance;
    }

    void add(CANConnection *) {}
    void remove(CANConnection *) {}
    QList<CANConnection *> &getConnections() { return mConnections; }
    int getNumBuses() { return 1; }
    void sendFrame(const CANFrame &) {}
    void framesReceived(CANConnection *, QList<CANFrame> &) {}

    static void registerMetaTypes() {}

private:
    CANConManager() = default;
    QList<CANConnection *> mConnections;
};
