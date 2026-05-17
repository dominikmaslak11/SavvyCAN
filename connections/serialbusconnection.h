#ifndef SERIALBUSCONNECTION_H
#define SERIALBUSCONNECTION_H

#include "canconnection.h"
#include "canframemodel.h"

#include <QCanBusDevice>
#include <QTimer>

/*
QCanBusDevice::UserKey
0x00000001 - enable silent mode
0x00000002 - enable loopback mode
0x00000004 - disable auto retransmissions
0x00000008 - enable terminator
0x00000010 - enable automatic bus off recovery
*/

#define EN_SILENT_MODE                  0x00000001
#define EN_LOOPBACK_MODE                0x00000002
#define DIS_AUTO_RETRANSMISSIONS        0x00000004
#define EN_TERMINATOR                   0x00000008
#define EN_AUTOMATIC_BUSOFF_RECOVERY    0x00000010

class SerialBusConnection : public CANConnection
{
    Q_OBJECT

public:
  SerialBusConnection(QString portName, QString driverName, int pBusSpeed,
		      int pDataRate, bool pCanFd);
    ~SerialBusConnection() override;

protected:

    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

    void disconnectDevice();

private slots:
    void errorReceived(QCanBusDevice::CanBusError) const;
    void framesWritten(qint64 count);
    void framesReceived();
    void testConnection();

protected:
    QCanBusDevice     *mDev_p = nullptr;
    QTimer             mTimer;
};


#endif // SERIALBUSCONNECTION_H
