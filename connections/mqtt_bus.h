#ifndef MQTTBUS_H
#define MQTTBUS_H

#include <QCanBusDevice>
#include <QThread>
#include <QTimer>
#include "mqtt/qmqtt.h"

/*************/
#include <QDateTime>
/*************/

#include "canframemodel.h"
#include "canconnection.h"
#include "canconmanager.h"
#include "simplecrypt.h"

class MQTT_BUS : public CANConnection
{
    Q_OBJECT

public:
    MQTT_BUS(QString topicName);
    ~MQTT_BUS() override;

protected:

    void piStarted() override;
    void piStop() override;
    void piSetBusSettings(int pBusIdx, CANBus pBus) override;
    bool piGetBusSettings(int pBusIdx, CANBus& pBus) override;
    void piSuspend(bool pSuspend) override;
    bool piSendFrame(const CANFrame&) override;

    void disconnectDevice();

private slots:
    void clientConnected();
    void clientErrored(const QMQTT::ClientError error);
    void clientMessageReceived(const QMQTT::Message& message);

private:
    void readSettings();
    void rebuildLocalTimeBasis();
    void sendDebug(const QString debugText);
    QString genRandomClientID();
    SimpleCrypt *crypto;

protected:
    QTimer             mTimer;
    QThread            mThread;

    QMQTT::Client *mqttClient;
    QString topicName;

    bool isAutoRestart;
    int framesRapid;
    CANFrame buildFrame;
    qint64 buildTimestamp;
    quint32 buildId;
    QByteArray buildData;
    uint32_t buildTimeBasis;
    int32_t timeBasis;
    uint64_t lastSystemTimeBasis;
};

#endif // MQTT_BUS_H
