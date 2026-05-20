#include "qserialport.h"

QList<QSerialPortInfo> QSerialPortInfo::availablePorts()
{
    return {}; // No serial ports on Android via QtSerialPort
}

// ── QSerialPort ───────────────────────────────────────────────────────────

QSerialPort::QSerialPort(QObject *parent) : QObject(parent) {}
QSerialPort::QSerialPort(const QString &name, QObject *parent) : QObject(parent), m_portName(name) {}
QSerialPort::QSerialPort(const QSerialPortInfo &info, QObject *parent) : QObject(parent), m_portName(info.portName()) {}
QSerialPort::~QSerialPort() = default;

bool QSerialPort::open(int) { m_isOpen = true; return true; }
void QSerialPort::close() { m_isOpen = false; }
bool QSerialPort::isOpen() const { return m_isOpen; }

bool QSerialPort::setBaudRate(qint32 rate, Directions) { m_baudRate = rate; return true; }
bool QSerialPort::setDataBits(QSerialPort::DataBits bits) { m_dataBits = bits; return true; }
bool QSerialPort::setParity(QSerialPort::Parity parity) { m_parity = parity; return true; }
bool QSerialPort::setStopBits(QSerialPort::StopBits stop) { m_stopBits = stop; return true; }
bool QSerialPort::setFlowControl(QSerialPort::FlowControl flow) { m_flowControl = flow; return true; }

qint32 QSerialPort::baudRate(Directions) const { return m_baudRate; }
QSerialPort::DataBits QSerialPort::dataBits() const { return m_dataBits; }
QSerialPort::Parity QSerialPort::parity() const { return m_parity; }
QSerialPort::StopBits QSerialPort::stopBits() const { return m_stopBits; }
QSerialPort::FlowControl QSerialPort::flowControl() const { return m_flowControl; }

void QSerialPort::setPortName(const QString &name) { m_portName = name; }
QString QSerialPort::portName() const { return m_portName; }
void QSerialPort::setPort(const QSerialPortInfo &info) { m_portName = info.portName(); }

qint64 QSerialPort::write(const QByteArray &) { return 0; }
qint64 QSerialPort::write(const char *, qint64) { return 0; }
QByteArray QSerialPort::read(qint64) { return {}; }
QByteArray QSerialPort::readAll() { return {}; }

qint64 QSerialPort::bytesAvailable() const { return 0; }
bool QSerialPort::waitForReadyRead(int) { return false; }
bool QSerialPort::waitForBytesWritten(int) { return false; }
bool QSerialPort::flush() { return true; }

QSerialPort::SerialPortError QSerialPort::error() const { return m_error; }
QString QSerialPort::errorString() const { return m_errorString; }
void QSerialPort::clearError() { m_error = NoError; m_errorString.clear(); }
void QSerialPort::clear(Directions) {}
