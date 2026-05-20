#ifndef QSERIALPORT_H
#define QSERIALPORT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QList>

class QSerialPortInfo
{
public:
    QSerialPortInfo() = default;
    QSerialPortInfo(const QString &name) : m_portName(name) {}
    QSerialPortInfo(const QSerialPortInfo &) = default;
    
    QString portName() const { return m_portName; }
    QString description() const { return m_description; }
    QString manufacturer() const { return m_manufacturer; }
    QString serialNumber() const { return m_serialNumber; }
    quint16 vendorIdentifier() const { return 0; }
    quint16 productIdentifier() const { return 0; }
    bool isValid() const { return !m_portName.isEmpty(); }
    bool isNull() const { return m_portName.isEmpty(); }
    bool isBusy() const { return false; }

    static QList<QSerialPortInfo> availablePorts();
    
    QString systemLocation() const { return m_portName; }

private:
    QString m_portName;
    QString m_description;
    QString m_manufacturer;
    QString m_serialNumber;
};

class QSerialPort : public QObject
{
    Q_OBJECT
public:
    enum Direction { Input = 1, Output = 2, AllDirections = Input | Output };
    typedef Direction Directions;
    
    enum BaudRate { Baud1200 = 1200, Baud2400 = 2400, Baud4800 = 4800, Baud9600 = 9600,
                    Baud19200 = 19200, Baud38400 = 38400, Baud57600 = 57600, Baud115200 = 115200 };
    enum DataBits { Data5 = 5, Data6 = 6, Data7 = 7, Data8 = 8 };
    enum Parity { NoParity = 0, EvenParity = 2, OddParity = 3, SpaceParity = 4, MarkParity = 5 };
    enum StopBits { OneStop = 1, OneAndHalfStop = 3, TwoStop = 2 };
    enum FlowControl { NoFlowControl = 0, HardwareControl = 1, SoftwareControl = 2 };
    enum PinoutSignal {};
    enum SerialPortError { NoError = 0, DeviceNotFoundError = 1, PermissionError = 2,
                           OpenError = 3, WriteError = 7, ReadError = 8, ResourceError = 9,
                           UnsupportedOperationError = 10, UnknownError = 11, TimeoutError = 12,
                           NotOpenError = 13 };

    explicit QSerialPort(QObject *parent = nullptr);
    explicit QSerialPort(const QString &name, QObject *parent = nullptr);
    explicit QSerialPort(const QSerialPortInfo &info, QObject *parent = nullptr);
    ~QSerialPort();

    bool open(int mode);
    void close();
    bool isOpen() const;
    
    bool setBaudRate(qint32 baudRate, Directions directions = AllDirections);
    bool setDataBits(DataBits dataBits);
    bool setParity(Parity parity);
    bool setStopBits(StopBits stopBits);
    bool setFlowControl(FlowControl flowControl);
    
    qint32 baudRate(Directions directions = AllDirections) const;
    DataBits dataBits() const;
    Parity parity() const;
    StopBits stopBits() const;
    FlowControl flowControl() const;
    
    void setPortName(const QString &name);
    QString portName() const;
    void setPort(const QSerialPortInfo &info);
    
    qint64 write(const QByteArray &data);
    qint64 write(const char *data, qint64 len);
    QByteArray read(qint64 maxSize);
    QByteArray readAll();
    
    qint64 bytesAvailable() const;
    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);
    bool flush();
    
    SerialPortError error() const;
    QString errorString() const;
    void clearError();
    
    void clear(Directions directions = AllDirections);

signals:
    void readyRead();
    void bytesWritten(qint64 bytes);
    void errorOccurred(SerialPortError error);

private:
    QString m_portName;
    bool m_isOpen = false;
    QByteArray m_readBuffer;
    SerialPortError m_error = NoError;
    QString m_errorString;
    qint32 m_baudRate = Baud115200;
    DataBits m_dataBits = Data8;
    Parity m_parity = NoParity;
    StopBits m_stopBits = OneStop;
    FlowControl m_flowControl = NoFlowControl;
};

#endif // QSERIALPORT_H
