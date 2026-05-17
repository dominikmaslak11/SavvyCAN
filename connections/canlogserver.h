#ifndef CANLOGSERVER_H
#define CANLOGSERVER_H

#include <stdio.h>

#include <QCanBusDevice>
#include <QThread>
#include <QTimer>
#include <QTcpSocket>

/*************/
#include <QDateTime>
/*************/

#include "canframemodel.h"
#include "canconnection.h"
#include "canconmanager.h"

class CanLogServer : public CANConnection
{
    Q_OBJECT

public:
    explicit CanLogServer(QString serverAddress);
    ~CanLogServer() override;

// Interface
protected:
    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

private slots:
    void readNetworkData();
    void networkConnected();
    void networkDisconnected();

// Utility
private:
    void readSettings();
    void connectToDevice();
    void disconnectFromDevice();
    void heartbeat();

// Attributes
protected:
     QTcpSocket *m_ptcpSocket = nullptr;
     QString m_qsAddress;

//    QUdpSocket *_udpClient;

//    QTimer  *_heartbeatTimer;
};
#endif // CANLOGSERVER_H
