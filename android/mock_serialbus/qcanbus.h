#ifndef QCANBUS_H
#define QCANBUS_H

#include "qcanbusdevice.h"
#include <QObject>
#include <QList>
#include <QString>

class QCanBus : public QObject
{
    Q_OBJECT
public:
    static QCanBus *instance();
    
    QList<QCanBusDeviceInfo> availableDevices(const QString &plugin, QString *errorMessage = nullptr) const;
    QCanBusDevice *createDevice(const QString &plugin, const QString &interfaceName, QString *errorMessage = nullptr);
    QStringList plugins() const;

private:
    QCanBus(QObject *parent = nullptr);
    static QCanBus *self;
};

#endif // QCANBUS_H
