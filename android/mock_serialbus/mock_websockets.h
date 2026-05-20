#ifndef MOCK_WEBSOCKETS_H
#define MOCK_WEBSOCKETS_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QHostAddress>

// Minimal stubs for QtWebSockets (not available on Android)

class QWebSocketServer : public QObject
{
    Q_OBJECT
public:
    enum SslMode { NonSecureMode, SecureMode };

    explicit QWebSocketServer(const QString &, SslMode, QObject *parent = nullptr) : QObject(parent) {}
    ~QWebSocketServer() = default;

    bool listen(const QHostAddress & = QHostAddress::Any, quint16 = 0) { return false; }
    void close() {}
    bool isListening() const { return false; }
    quint16 serverPort() const { return 0; }

    class QWebSocket *nextPendingConnection() { return nullptr; }

signals:
    void newConnection();
};

class QWebSocket : public QObject
{
    Q_OBJECT
public:
    explicit QWebSocket(QObject *parent = nullptr) : QObject(parent) {}
    ~QWebSocket() = default;

    void open(const QUrl &) {}
    void close() {}
    void ping() {}
    qint64 sendTextMessage(const QString &) { return 0; }
    qint64 sendBinaryMessage(const QByteArray &) { return 0; }
    bool isValid() const { return false; }

signals:
    void connected();
    void disconnected();
    void textMessageReceived(const QString &);
    void binaryMessageReceived(const QByteArray &);
    void error();
};

#endif // MOCK_WEBSOCKETS_H
