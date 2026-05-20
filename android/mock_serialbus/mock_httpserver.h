#ifndef MOCK_HTTPSERVER_H
#define MOCK_HTTPSERVER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QByteArray>
#include <QHostAddress>

// Minimal stubs for QtHttpServer (not available on Android)
namespace QtHttpServer {

class QHttpServerRequest
{
public:
    enum Method { Get, Post, Put, Delete, Patch, Head, Options, Unknown };

    QHttpServerRequest() = default;
    Method method() const { return Unknown; }
    QUrl url() const { return {}; }
    QString path() const { return {}; }
    QByteArray body() const { return {}; }
    QString header(const QString &) const { return {}; }
    QByteArray query() const { return {}; }
};

class QHttpServerResponse
{
public:
    enum StatusCode { Ok = 200, BadRequest = 400, NotFound = 404 };

    QHttpServerResponse() = default;
    QHttpServerResponse(const QString &, StatusCode = Ok) {}
    QHttpServerResponse(StatusCode, const QByteArray & = {}) {}
    StatusCode statusCode() const { return Ok; }
    QByteArray body() const { return {}; }
};

class QHttpServer : public QObject
{
    Q_OBJECT
public:
    explicit QHttpServer(QObject *parent = nullptr) : QObject(parent) {}
    ~QHttpServer() = default;

    void route(const QString &, const QByteArray &, ...) {}
    bool listen(const QHostAddress & = QHostAddress::Any, quint16 = 0) { return false; }
    void close() {}
    bool isListening() const { return false; }
    quint16 serverPort() const { return 0; }
};

} // namespace QtHttpServer

#endif // MOCK_HTTPSERVER_H
