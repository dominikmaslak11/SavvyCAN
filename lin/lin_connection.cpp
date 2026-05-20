#include "lin_connection.h"
#include <QDebug>
#include <QtGlobal>
#include <QThread>

namespace {
QString stripLineEnding(QString line)
{
    while (!line.isEmpty() && (line.endsWith('\r') || line.endsWith('\n'))) {
        line.chop(1);
    }
    return line;
}
}

// ═══════════════════════════════════════════════════════════════════════════
// LINConnection base class
// ═══════════════════════════════════════════════════════════════════════════

LINConnection::LINConnection(QString port,
                             LINCon::type type,
                             int baudRate,
                             int numBuses,
                             int queueLen)
    : mNumBuses(numBuses)
    , mPort(port)
    , mType(type)
    , mBaudRate(baudRate)
    , mStatus(LINCon::NOT_CONNECTED)
{
    mQueue.setSize(queueLen);
}

LINConnection::~LINConnection()
{
    if (mStarted)
        stop();
}

LINCon::status LINConnection::getStatus() const noexcept
{
    return static_cast<LINCon::status>(mStatus.loadAcquire());
}

void LINConnection::setStatus(LINCon::status s)
{
    mStatus.storeRelease(s);
    emit statusChanged(s);
}

void LINConnection::start()
{
    if (mStarted) return;

    mThread_p = new QThread();
    connect(mThread_p, &QThread::started, this, [this]() {
        piStarted();
        setStatus(LINCon::CONNECTED);
    });
    connect(mThread_p, &QThread::finished, this, [this]() {
        setStatus(LINCon::NOT_CONNECTED);
    });

    this->moveToThread(mThread_p);
    mThread_p->start();
    mStarted = true;
}

void LINConnection::stop()
{
    if (!mStarted) return;

    piStop();
    mThread_p->quit();
    mThread_p->wait(1000);
    delete mThread_p;
    mThread_p = nullptr;
    mStarted = false;
    setStatus(LINCon::NOT_CONNECTED);
}

bool LINConnection::sendFrame(const LINFrame &frame)
{
    if (getStatus() != LINCon::CONNECTED) return false;
    return piSendFrame(frame);
}

bool LINConnection::sendHeader(uint8_t id, int bus)
{
    if (getStatus() != LINCon::CONNECTED) return false;
    return piSendHeader(id, bus);
}

void LINConnection::suspend(bool pSuspend)
{
    mCapSuspended = pSuspend;
    piSuspend(pSuspend);
}

// ═══════════════════════════════════════════════════════════════════════════
// LINSerialConnection
// ═══════════════════════════════════════════════════════════════════════════

LINSerialConnection::LINSerialConnection(QString port, int baudRate)
    : LINConnection(port, LINCon::LIN_SERIAL, baudRate, 1, 1024)
{
}

LINSerialConnection::~LINSerialConnection()
{
    stop();
}

QList<QSerialPortInfo> LINSerialConnection::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

void LINSerialConnection::piStarted()
{
    mLucMode = false;
    mSerial = new QSerialPort();
    mSerial->setPortName(mPort);
    mSerial->setBaudRate(mBaudRate);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setFlowControl(QSerialPort::NoFlowControl);

    if (!mSerial->open(QIODevice::ReadWrite)) {
        LINConnection::error(QString("Cannot open serial port: %1 — %2")
                       .arg(mPort)
                       .arg(mSerial->errorString()));
        return;
    }

    connect(mSerial, &QSerialPort::readyRead, this, &LINSerialConnection::onReadyRead);
    connect(mSerial, &QSerialPort::errorOccurred, this, &LINSerialConnection::onError);

    mTimer.start();

    if (tryInitializeLuc()) {
        mLucMode = true;
        if (mConsoleOutput)
            qDebug() << "[LIN] Detected uCANDevices LUC protocol on" << mPort;
    } else {
        if (mConsoleOutput)
            qDebug() << "[LIN] Using raw LIN serial mode on" << mPort;
    }

    if (mConsoleOutput)
        qDebug() << "[LIN] Serial port opened:" << mPort << "baud:" << mBaudRate;
}

void LINSerialConnection::piStop()
{
    if (mSerial && mSerial->isOpen() && mLucMode) {
        sendLucCommand("r2ff0\r");
        sendLucCommand("C\r");
    }
    if (mSerial) {
        mSerial->close();
        delete mSerial;
        mSerial = nullptr;
    }
    mRxBuffer.clear();
}

bool LINSerialConnection::piSendFrame(const LINFrame &frame)
{
    if (!mSerial || !mSerial->isOpen())
        return false;

    if (mLucMode) {
        const uint8_t id = frame.id & LIN::PID_MASK_ID;
        QByteArray command;
        command += 't';
        command += '0';
        command += QByteArray::number(id, 16).rightJustified(2, '0').toUpper();
        command += QByteArray::number(qBound(0, static_cast<int>(frame.dataLen), LIN::MAX_DATA_LEN));
        for (int i = 0; i < qMin<int>(frame.dataLen, LIN::MAX_DATA_LEN); ++i) {
            command += QByteArray::number(static_cast<uint8_t>(frame.data[i]), 16).rightJustified(2, '0').toUpper();
        }
        command += '\r';
        return sendLucCommand(command);
    }

    QByteArray raw = LINHelpers::encodeRawFrame(frame);
    qint64 written = mSerial->write(raw);
    return written == raw.size();
}

bool LINSerialConnection::piSendHeader(uint8_t id, int bus)
{
    Q_UNUSED(bus)
    if (!mSerial || !mSerial->isOpen())
        return false;

    if (mLucMode) {
        QByteArray command;
        command += 'r';
        command += '0';
        command += QByteArray::number(id & LIN::PID_MASK_ID, 16).rightJustified(2, '0').toUpper();
        command += '8';
        command += '\r';
        return sendLucCommand(command);
    }

    // LIN header: Break field + Sync (0x55) + PID
    // Break field is typically generated by the LIN transceiver hardware
    // when the master sends 0x00 at a lower baud rate or toggles break.
    // For simple serial adapters, we send the break by writing 0x00 with
    // a break condition if the serial port supports it.
    //
    // For a basic USB-serial LIN adapter:
    QByteArray header;
    // Attempt break condition (implementation-dependent)
    mSerial->setBreakEnabled(true);
    QThread::usleep(1000); // ~13 bit times at 19200
    mSerial->setBreakEnabled(false);

    // Sync byte
    header.append((char)LIN::SYNC_BYTE);
    // Protected ID
    header.append((char)LINHelpers::buildPID(id));

    mSerial->write(header);
    return true;
}

void LINSerialConnection::piSuspend(bool pSuspend)
{
    Q_UNUSED(pSuspend)
    // For serial, nothing special needed
}

void LINSerialConnection::onReadyRead()
{
    mRxBuffer.append(mSerial->readAll());
    processBuffer();
}

void LINSerialConnection::onError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    qWarning() << "[LIN] Serial error:" << error << mSerial->errorString();
    LINConnection::error(QString("Serial error: %1").arg(mSerial->errorString()));
}

void LINSerialConnection::processBuffer()
{
    if (mLucMode) {
        processLucBuffer();
        return;
    }

    // Simple LIN frame parser from raw serial stream.
    // This assumes the LIN adapter strips the break/sync fields and
    // delivers frame data as: [PID] [D0..D7] [Checksum]
    //
    // For adapters that pass break/sync, we'd need to sync on break.
    // We'll look for 0x55 (sync byte) as a frame delimiter.

    while (mRxBuffer.size() >= 3) { // Minimum frame: PID + 1 data + checksum
        int frameLen = 0;

        // Try to detect frame structure
        // If adapter strips break+sync, frame starts with PID
        uint8_t pid = (uint8_t)mRxBuffer[0];

        // Check if this looks like a valid PID (parity bits make sense)
        if (LINHelpers::verifyParity(pid)) {
            // Count data bytes by scanning for checksum
            // Data length is not encoded in the frame; we need to know it externally.
            // For a first implementation, we try lengths 1-8 and pick the one
            // where the checksum matches.
            bool found = false;
            for (int tryLen = 1; tryLen <= LIN::MAX_DATA_LEN && (tryLen + 2) <= mRxBuffer.size(); ++tryLen) {
                LINFrame frame;
                QByteArray candidate = mRxBuffer.left(tryLen + 2);
                if (LINHelpers::decodeRawFrame(candidate, frame)) {
                    // Found a valid frame — accept it
                    frame.timestamp = mTimer.nsecsElapsed() / 1000; // microseconds
                      LINFrame* p = mQueue.get(); if (p) { *p = frame; mQueue.queue(); }

                    QVector<LINFrame> batch;
                    batch.append(frame);
                    emit framesReceived(frame.bus, batch);

                    mRxBuffer.remove(0, tryLen + 2);
                    found = true;
                    break;
                }
            }
            if (!found) {
                // No valid checksum found; skip one byte and retry
                mRxBuffer.remove(0, 1);
            }
        } else {
            // Not a valid PID; skip this byte
            mRxBuffer.remove(0, 1);
        }
    }
}

bool LINSerialConnection::tryInitializeLuc()
{
    QByteArray response;
    if (!sendLucCommand("v\r", &response, 300)) {
        return false;
    }

    if (!response.startsWith('v')) {
        return false;
    }

    const QByteArray speedCmd = (mBaudRate <= LIN::BAUD_9600) ? QByteArray("S2\r")
                                                               : QByteArray("S1\r");
    sendLucCommand(speedCmd);
    sendLucCommand("l\r");
    sendLucCommand("r1ff0\r");
    return true;
}

bool LINSerialConnection::sendLucCommand(const QByteArray &command, QByteArray *response, int timeoutMs)
{
    if (!mSerial || !mSerial->isOpen())
        return false;

    if (mSerial->write(command) != command.size()) {
        return false;
    }

    if (!response)
        return true;

    QByteArray data;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        if (mSerial->waitForReadyRead(25)) {
            data.append(mSerial->readAll());
            int end = data.indexOf('\r');
            if (end >= 0) {
                *response = data.left(end + 1);
                return true;
            }
        }
    }

    *response = data;
    return !data.isEmpty();
}

void LINSerialConnection::processLucBuffer()
{
    while (true) {
        int end = mRxBuffer.indexOf('\r');
        if (end < 0)
            break;

        QString line = QString::fromLatin1(mRxBuffer.left(end));
        mRxBuffer.remove(0, end + 1);
        line = stripLineEnding(line.trimmed());
        if (line.isEmpty())
            continue;

        if (!line.startsWith('t') || line.length() < 6)
            continue;

        bool ok = false;
        const uint8_t id = static_cast<uint8_t>(line.mid(2, 2).toUInt(&ok, 16));
        if (!ok)
            continue;

        const int dataLen = line.mid(4, 1).toInt(&ok, 16);
        if (!ok || dataLen < 0 || dataLen > LIN::MAX_DATA_LEN)
            continue;

        const QString dataStr = line.mid(5);
        if (dataStr.size() < dataLen * 2)
            continue;

        LINFrame frame;
        frame.id = id;
        frame.pid = LINHelpers::buildPID(id);
        frame.dataLen = static_cast<uint8_t>(dataLen);
        frame.isReceived = true;
        frame.timestamp = mTimer.nsecsElapsed() / 1000;
        frame.checksumType = LINChecksumType::Enhanced;

        for (int i = 0; i < dataLen; ++i) {
            frame.data[i] = static_cast<uint8_t>(dataStr.mid(i * 2, 2).toUInt(&ok, 16));
            if (!ok)
                break;
        }

        frame.calcChecksum = LINHelpers::calcEnhancedChecksum(frame.pid, frame.data, frame.dataLen);
        frame.checksum = frame.calcChecksum;

        LINFrame *slot = mQueue.get();
        if (slot) {
            *slot = frame;
            mQueue.queue();
        }

        QVector<LINFrame> batch;
        batch.append(frame);
        emit framesReceived(frame.bus, batch);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// LINSocketCANConnection (Linux only)
// ═══════════════════════════════════════════════════════════════════════════

#ifdef Q_OS_LINUX

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <linux/can.h>
// LIN support in SocketCAN is via CAN_RAW with LIN filter rules (since kernel 6.x)
// or via dedicated LIN protocol family (CAN_LIN, proposed but not yet merged).
// For now, we use CAN_RAW with recvfrom and manually decode LIN frames
// from the CAN FD messages that wrap LIN data.

LINSocketCANConnection::LINSocketCANConnection(QString interfaceName)
    : LINConnection(interfaceName, LINCon::LIN_SOCKETCAN, LIN::BAUD_19200, 1, 1024)
    , mInterfaceName(interfaceName)
{
}

LINSocketCANConnection::~LINSocketCANConnection()
{
    piStop();
}

void LINSocketCANConnection::piStarted()
{
    mSocketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (mSocketFd < 0) {
        LINConnection::error("Cannot create CAN socket");
        return;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, mInterfaceName.toUtf8().constData(), IFNAMSIZ - 1);
    if (ioctl(mSocketFd, SIOCGIFINDEX, &ifr) < 0) {
        LINConnection::error(QString("Cannot find interface: %1").arg(mInterfaceName));
        close(mSocketFd);
        mSocketFd = -1;
        return;
    }

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(mSocketFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LINConnection::error("Cannot bind CAN socket");
        close(mSocketFd);
        mSocketFd = -1;
        return;
    }

    // Enable CAN FD to capture LIN-wrapped frames if present
    int can_fd = 1;
    #ifdef CAN_RAW_FD_FRAMES
    setsockopt(mSocketFd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &can_fd, sizeof(can_fd));
#endif

    // Set non-blocking
    int flags = fcntl(mSocketFd, F_GETFL, 0);
    fcntl(mSocketFd, F_SETFL, flags | O_NONBLOCK);

    mTimer.start();
}

void LINSocketCANConnection::piStop()
{
    if (mSocketFd >= 0) {
        close(mSocketFd);
        mSocketFd = -1;
    }
}

bool LINSocketCANConnection::piSendFrame(const LINFrame &frame)
{
    Q_UNUSED(frame)
    // SocketCAN LIN transmission is hardware-dependent
    return false;
}

bool LINSocketCANConnection::piSendHeader(uint8_t id, int bus)
{
    Q_UNUSED(id)
    Q_UNUSED(bus)
    return false;
}

void LINSocketCANConnection::piSuspend(bool pSuspend)
{
    Q_UNUSED(pSuspend)
}

#endif // Q_OS_LINUX
