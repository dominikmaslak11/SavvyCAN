#ifndef SOCKETCAND_H
#define SOCKETCAND_H

//#include <QSerialPort>
//#include <QCanBusDevice>
#include <QTimer>
#include <QTcpSocket>
#include <QHostAddress>
//#include <QUdpSocket>

/*************/
#include <QDateTime>
/*************/

#include "canframemodel.h"
#include "canconnection.h"
#include "canconmanager.h"


namespace KAYAKSTATE {

enum MODE
{
    IDLE, //NO_BUS in socketcand doc
    BCM,
    SWITCHING2RAW,
    RAWMODE,
    ISOTP
};

}

using namespace KAYAKSTATE;
class SocketCANd : public CANConnection
{
    Q_OBJECT

public:
    explicit SocketCANd(QString portName);
    ~SocketCANd() override;

protected:

    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

    void disconnectDevice();

private slots:
    void connectDevice();
    void checkConnection();
    void readTCPData(int busNum);
    void invokeReadTCPData();
    void deviceConnected(int busNum);
    void switchToRawMode(int busNum);
    QString decodeFrames(QString, int busNum);

private:
    void procRXData(QString, int busNum);
    void sendBytesToTCP(const QByteArray &bytes, int busNum);
    void sendStringToTCP(const char* data, int busNum);
    void sendDebug(const QString debugText);

protected:
    QTimer mTimer;
    bool reconnecting;
    QVarLengthArray<QTcpSocket*> tcpClient;
    QHostAddress hostIP;
    int hostPort;
    QList<QString> hostCanIDs;
    int framesRapid;
    QVarLengthArray<MODE> rx_state;
    CANFrame buildFrame;
    QVarLengthArray<QString> unprocessedData;
};


#endif // SOCKETCAND_H
