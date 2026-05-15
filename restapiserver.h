#ifndef RESTAPISERVER_H
#define RESTAPISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QTcpServer>

class FrameStore;
class MainWindow;

/// RestApiServer exposes the SavvyCAN frame data and controls
/// via a local HTTP REST API.  Built on QtHttpServer (Qt 6.4+).
///
/// Endpoints:
///   GET  /api/status           — connection & frame count summary
///   GET  /api/frames?limit=100&id=0x7E0  — recent frames (JSON)
///   GET  /api/buses            — active bus list
///   POST /api/send             — send a CAN frame (JSON body)
///   GET  /api/dbc/signals/:id  — DBC signals for a frame ID
class RestApiServer : public QObject
{
    Q_OBJECT

public:
    explicit RestApiServer(FrameStore *store, QObject *parent = nullptr);
    ~RestApiServer() = default;

    /// Start listening on the given port.  Returns true on success.
    bool start(quint16 port = 8080);

    /// Stop the server.
    void stop();

    /// Whether the server is currently listening.
    bool isRunning() const;

    /// Current port number.
    quint16 port() const;

signals:
    void started(quint16 port);
    void stopped();
    void requestLog(const QString &entry);

private:
    void setupRoutes();

    FrameStore  *mStore;
    QHttpServer  mServer;
    QTcpServer   mTcpServer;
    quint16      mPort = 0;
};

#endif // RESTAPISERVER_H
