#ifndef LIN_CONNECTION_H
#define LIN_CONNECTION_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QElapsedTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "lin_structs.h"
#include "utils/lfqueue.h"

// ═══════════════════════════════════════════════════════════════════════════
// LIN-specific connection type enumeration
// ═══════════════════════════════════════════════════════════════════════════

namespace LINCon {
    enum type {
        NONE = 0,
        LIN_SERIAL,       // Direct serial LIN adapter (e.g., MCP2004, LIN-USB)
        LIN_SOCKETCAN,    // Linux SocketCAN LIN driver (vcan / slcan with LIN)
        LIN_LAWICEL,      // Lawicel / SLCAN-style LIN adapter
        LIN_GVRET         // GVRET firmware with LIN support
    };

    enum status {
        NOT_CONNECTED,
        CONNECTED
    };
}

// ═══════════════════════════════════════════════════════════════════════════
// LINConnection — base class for all LIN bus connections
// ═══════════════════════════════════════════════════════════════════════════

class LINConnection : public QObject
{
    Q_OBJECT

protected:
    LINConnection(QString port,
                  LINCon::type type,
                  int baudRate,
                  int numBuses,
                  int queueLen);

public:
    virtual ~LINConnection();

    int getNumBuses() const noexcept { return mNumBuses; }
    QString getPort() const noexcept { return mPort; }
    LINCon::type getType() const noexcept { return mType; }
    LINCon::status getStatus() const noexcept;
    LFQueue<LINFrame>& getQueue() noexcept { return mQueue; }

signals:
    void error(const QString &msg);
    void statusChanged(LINCon::status newStatus);
    void debugOutput(const QString &msg);
    void framesReceived(int bus, QVector<LINFrame> frames);

public slots:
    void start();
    void stop();
    bool sendFrame(const LINFrame &frame);
    bool sendHeader(uint8_t id, int bus = 0);  // Master sends header
    void suspend(bool pSuspend);

protected:
    virtual void piStarted() = 0;
    virtual void piStop() = 0;
    virtual bool piSendFrame(const LINFrame &frame) = 0;
    virtual bool piSendHeader(uint8_t id, int bus) = 0;
    virtual void piSuspend(bool pSuspend) = 0;

    void setStatus(LINCon::status s);

    int              mNumBuses;
    LFQueue<LINFrame> mQueue;
    const QString     mPort;
    const LINCon::type mType;
    int               mBaudRate;
    bool              mConsoleOutput = false;
    QElapsedTimer      mTimer;

private:
    QAtomicInt        mStatus;
    QThread*          mThread_p = nullptr;
    bool              mStarted = false;
    bool              mCapSuspended = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// LINSerialConnection — raw serial LIN adapter
// ═══════════════════════════════════════════════════════════════════════════

class LINSerialConnection : public LINConnection
{
    Q_OBJECT

public:
    LINSerialConnection(QString port, int baudRate = LIN::BAUD_19200);
    virtual ~LINSerialConnection();

    static QList<QSerialPortInfo> availablePorts();

signals:
    void rawBytesReceived(QByteArray data);

protected:
    void piStarted() override;
    void piStop() override;
    bool piSendFrame(const LINFrame &frame) override;
    bool piSendHeader(uint8_t id, int bus) override;
    void piSuspend(bool pSuspend) override;

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError err);

private:
    void processBuffer();

    QSerialPort* mSerial = nullptr;
    QByteArray   mRxBuffer;
    QElapsedTimer mTimer;
    int           mTimeout_ms = 100;
};

// ═══════════════════════════════════════════════════════════════════════════
// SocketCAN LIN connection (Linux-specific)
// ═══════════════════════════════════════════════════════════════════════════

#ifdef Q_OS_LINUX

class LINSocketCANConnection : public LINConnection
{
    Q_OBJECT

public:
    LINSocketCANConnection(QString interfaceName);
    virtual ~LINSocketCANConnection();

protected:
    void piStarted() override;
    void piStop() override;
    bool piSendFrame(const LINFrame &frame) override;
    bool piSendHeader(uint8_t id, int bus) override;
    void piSuspend(bool pSuspend) override;

private:
    int mSocketFd = -1;
    QString mInterfaceName;
};

#endif // Q_OS_LINUX

#endif // LIN_CONNECTION_H
