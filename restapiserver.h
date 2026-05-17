#ifndef RESTAPISERVER_H
#define RESTAPISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QTcpServer>
#include <QWebSocketServer>
#include <QWebSocket>

class FrameStore;
class MainWindow;
class PythonBridge;

/// RestApiServer exposes the SavvyCAN frame data and controls
/// via a local HTTP REST API.  Built on QtHttpServer (Qt 6.4+).
///
/// Endpoints:
///   GET  /api/status           — connection & frame count summary
///   GET  /api/frames?limit=100&id=0x7E0  — recent frames (JSON)
///   GET  /api/buses            — active bus list
///   POST /api/send             — send a CAN frame (JSON body)
///   GET  /api/dbc/signals/:id  — DBC signals for a frame ID
///   POST /api/connect          — connect to a CAN interface (JSON body)
///   POST /api/python           — execute Python code via embedded interpreter
///   WS   /api/ws/live          — WebSocket live frame stream
class RestApiServer : public QObject
{
    Q_OBJECT

public:
    explicit RestApiServer(FrameStore *store, QObject *parent = nullptr);
    ~RestApiServer() = default;

    /// Set the Python bridge for /api/python execution
    void setPythonBridge(PythonBridge *bridge);

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
    PythonBridge *mPythonBridge = nullptr;
    QHttpServer  mServer;
    QTcpServer   mTcpServer;
    QWebSocketServer *mWsServer = nullptr;
    QVector<QWebSocket *> mWsClients;
    quint16      mPort = 0;
};

#endif // RESTAPISERVER_H
