#ifndef CANCONMANAGER_H
#define CANCONMANAGER_H

#include <QObject>
#include <QList>
#include "can_structs.h"

class CANConnection;

/// Minimal stub for CANConManager — provides symbols needed by bus_protocols.
/// The real Android connections use BluetoothManager/WifiManager/UsbHostManager.
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
    int getNumBuses() const { return 1; }
    void sendFrame(const CANFrame &) {}
    static void registerMetaTypes() {}

signals:
    void framesReceived(CANConnection *, QList<CANFrame> &);
    void updated(int);

private:
    CANConManager() = default;
    QList<CANConnection *> mConnections;
};

#endif // CANCONMANAGER_H
