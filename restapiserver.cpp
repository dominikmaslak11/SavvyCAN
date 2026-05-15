#include "restapiserver.h"
#include "framestore.h"
#include "dbc/dbchandler.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QWebSocketServer>
#include <QWebSocket>

RestApiServer::RestApiServer(FrameStore *store, QObject *parent)
    : QObject(parent), mStore(store)
{
    setupRoutes();
}

bool RestApiServer::start(quint16 port)
{
    mPort = port;
    mTcpServer.listen(QHostAddress::Any, port);
    if (!mTcpServer.isListening()) {
        qWarning() << "RestApiServer: failed to listen on port" << port;
        return false;
    }

    mServer.bind(&mTcpServer);

    // ── WebSocket live stream ──────────────────────────────────────
    mWsServer = new QWebSocketServer("SavvyCAN Live", QWebSocketServer::NonSecureMode, this);
    if (mWsServer->listen(QHostAddress::Any, port + 1)) {
        connect(mWsServer, &QWebSocketServer::newConnection, this, [this] {
            while (mWsServer->hasPendingConnections()) {
                auto *ws = mWsServer->nextPendingConnection();
                mWsClients.append(ws);
                connect(ws, &QWebSocket::disconnected, this, [this, ws] {
                    mWsClients.removeAll(ws);
                    ws->deleteLater();
                });
                emit requestLog("WebSocket client connected");
            }
        });
        emit requestLog(QString("REST API http://localhost:%1 | WebSocket ws://localhost:%2/api/ws/live").arg(mPort).arg(mPort + 1));
    }

    // Connect FrameStore signal to broadcast to WebSocket clients
    connect(mStore, &FrameStore::framesAppended, this, [this](int count) {
        if (mWsClients.isEmpty() || count > 100) return; // throttle for bulk
        auto frames = mStore->allFrames();
        // Send only the newest frames
        int start = qMax(0, frames.size() - qMin(count, 10));
        for (int i = start; i < frames.size(); ++i) {
            QJsonObject fo;
            fo["id"] = QString("0x%1").arg(frames[i].frameId(), 0, 16).toUpper();
            fo["bus"] = frames[i].bus;
            fo["timestamp_us"] = static_cast<qint64>(frames[i].timeStamp().microSeconds());
            QJsonArray data;
            for (int j = 0; j < frames[i].payload().size(); ++j)
                data.append(static_cast<uint8_t>(frames[i].payload()[j]));
            fo["data"] = data;
            fo["dlc"] = frames[i].payload().size();

            QJsonDocument doc(fo);
            for (auto *ws : mWsClients) {
                if (ws->state() == QAbstractSocket::ConnectedState)
                    ws->sendTextMessage(doc.toJson(QJsonDocument::Compact));
            }
        }
    });

    emit started(mPort);
    return true;
}

void RestApiServer::stop()
{
    if (mWsServer) {
        mWsServer->close();
        for (auto *ws : mWsClients) ws->deleteLater();
        mWsClients.clear();
    }
    mTcpServer.close();
    emit stopped();
}

bool RestApiServer::isRunning() const
{
    return mTcpServer.isListening();
}

quint16 RestApiServer::port() const
{
    return mPort;
}

void RestApiServer::setupRoutes()
{
    // ── GET /api/status ──────────────────────────────────────────────
    mServer.route("/api/status", [this]() {
        QJsonObject obj;
        obj["frame_count"] = mStore->frameCount();
        obj["filtered_count"] = mStore->filteredFrameCount();
        obj["api_version"] = "1.0";
        obj["app_version"] = "223";
        emit requestLog("GET /api/status");
        return QHttpServerResponse(obj);
    });

    // ── GET /api/frames ──────────────────────────────────────────────
    mServer.route("/api/frames", [this](const QHttpServerRequest &req) {
        const auto query = req.query();
        int limit = query.queryItemValue("limit").toInt();
        if (limit <= 0 || limit > 10000) limit = 100;

        QString idFilter = query.queryItemValue("id");
        bool hasIdFilter = !idFilter.isEmpty();
        uint32_t filterId = hasIdFilter ? idFilter.toUInt(nullptr, 16) : 0;

        auto frames = mStore->allFrames();
        QJsonArray arr;

        for (int i = frames.size() - 1; i >= 0 && arr.size() < limit; --i) {
            const auto &f = frames[i];
            if (hasIdFilter && f.frameId() != filterId) continue;

            QJsonObject fo;
            fo["id"] = QString("0x%1").arg(f.frameId(), 0, 16).toUpper();
            fo["extended"] = f.hasExtendedFrameFormat();
            fo["bus"] = f.bus;
            fo["timestamp_us"] = static_cast<qint64>(f.timeStamp().microSeconds());
            fo["is_rx"] = f.isReceived;

            QJsonArray data;
            const auto &payload = f.payload();
            for (int j = 0; j < payload.size(); ++j)
                data.append(static_cast<uint8_t>(payload[j]));
            fo["data"] = data;
            fo["dlc"] = payload.size();

            arr.append(fo);
        }

        emit requestLog(QString("GET /api/frames?limit=%1 → %2 results").arg(limit).arg(arr.size()));
        QJsonObject response;
        response["count"] = arr.size();
        response["frames"] = arr;
        return QHttpServerResponse(response);
    });

    // ── GET /api/buses ───────────────────────────────────────────────
    mServer.route("/api/buses", [this]() {
        QJsonArray arr;
        auto frames = mStore->allFrames();
        QSet<int> seenBuses;
        for (const auto &f : frames) {
            if (!seenBuses.contains(f.bus)) {
                seenBuses.insert(f.bus);
                QJsonObject b;
                b["bus"] = f.bus;
                arr.append(b);
            }
        }
        emit requestLog("GET /api/buses");
        QJsonObject response;
        response["buses"] = arr;
        return QHttpServerResponse(response);
    });

    // ── POST /api/send ───────────────────────────────────────────────
    mServer.route("/api/send", [this](const QHttpServerRequest &req) {
        if (req.method() != QHttpServerRequest::Method::Post) {
            return QHttpServerResponse(QHttpServerResponse::StatusCode::MethodNotAllowed);
        }

        QJsonDocument doc = QJsonDocument::fromJson(req.body());
        if (!doc.isObject()) {
            return QHttpServerResponse(QHttpServerResponse::StatusCode::BadRequest);
        }

        QJsonObject body = doc.object();
        uint32_t id = body["id"].toInteger();
        int bus = body["bus"].toInt(0);
        QJsonArray dataArr = body["data"].toArray();

        CANFrame frame;
        frame.setFrameId(id);
        if (id > 0x7FF) frame.setExtendedFrameFormat(true);
        frame.bus = bus;
        frame.isReceived = false;
        frame.setFrameType(QCanBusFrame::DataFrame);

        QByteArray payload;
        for (auto v : dataArr)
            payload.append(static_cast<char>(v.toInt()));
        frame.setPayload(payload);

        mStore->addFrame(frame);

        emit requestLog(QString("POST /api/send id=0x%1 bus=%2 len=%3")
            .arg(id, 0, 16).arg(bus).arg(payload.size()));

        QJsonObject response;
        response["status"] = "ok";
        return QHttpServerResponse(response);
    });

    // ── GET /api/dbc/signals/<id> ─────────────────────────────────────
    mServer.route("/api/dbc/signals/<arg>", [this](const QString &frameIdStr) {
        uint32_t id = frameIdStr.toUInt(nullptr, 16);
        auto *dbc = DBCHandler::getReference();
        QJsonArray arr;

        if (dbc) {
            CANFrame frame;
            frame.setFrameId(id);
            DBC_MESSAGE *msg = dbc->findMessage(frame);
            if (msg) {
                for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
                    DBC_SIGNAL *sig = msg->sigHandler->findSignalByIdx(j);
                    if (!sig) continue;
                    QJsonObject s;
                    s["name"] = sig->name;
                    s["start_bit"] = sig->startBit;
                    s["size"] = sig->signalSize;
                    s["factor"] = sig->factor;
                    s["bias"] = sig->bias;
                    s["signed"] = (sig->valType == SIGNED_INT);
                    s["little_endian"] = sig->intelByteOrder;
                    s["unit"] = sig->unitName;
                    s["comment"] = sig->comment;
                    arr.append(s);
                }
            }
        }

        emit requestLog(QString("GET /api/dbc/signals/0x%1 → %2 signals").arg(id, 0, 16).arg(arr.size()));
        QJsonObject response;
        response["frame_id"] = frameIdStr;
        response["signals"] = arr;
        return QHttpServerResponse(response);
    });
}
