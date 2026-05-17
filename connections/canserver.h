//
//  canserver.h
//  SavvyCAN
//
//  Created by Chris Whiteford on 2022-01-21.
//

#ifndef canserver_h
#define canserver_h

#include <stdio.h>

#include <QCanBusDevice>
#include <QThread>
#include <QTimer>
#include <QUdpSocket>

/*************/
#include <QDateTime>
/*************/

#include "canframemodel.h"
#include "canconnection.h"
#include "canconmanager.h"

class CANserver : public CANConnection
{
    Q_OBJECT

public:
    CANserver(QString serverAddress);
    ~CANserver() override;

protected:

    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;
    
private slots:
    void readNetworkData();
    void heartbeatTimerSlot();


private:
    void readSettings();
    
    void connectToDevice();
    void disconnectFromDevice();

    void heartbeat();
    
protected:
    QHostAddress _canserverAddress;
    
    QUdpSocket *_udpClient;

    QTimer  *_heartbeatTimer;
};

#endif /* canserver_h */
