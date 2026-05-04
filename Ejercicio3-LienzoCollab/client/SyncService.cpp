#include "SyncService.h"
#include "DrawingModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QWebSocket>

namespace {

constexpr char kJsonContentType[] = "application/json";

} // namespace

SyncService::SyncService(QObject *parent) : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    const QNetworkProxy noProxy(QNetworkProxy::NoProxy);
    m_nam->setProxy(noProxy);
    connect(m_nam, &QNetworkAccessManager::finished, this, &SyncService::onHttpFinished);

    m_ws = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    m_ws->setProxy(noProxy);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &SyncService::onWsTextMessage);
    connect(m_ws, &QWebSocket::connected, this, &SyncService::onWsConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &SyncService::onWsDisconnected);
}

SyncService::~SyncService() = default;

void SyncService::setBaseHttpUrl(const QUrl &url)
{
    m_httpBase = url;
}

QUrl SyncService::wsUrlFromHttp(const QUrl &http) const
{
    QUrl u = http;
    if (u.scheme() == QStringLiteral("http"))
        u.setScheme(QStringLiteral("ws"));
    else if (u.scheme() == QStringLiteral("https"))
        u.setScheme(QStringLiteral("wss"));
    u.setPath(QStringLiteral("/ws"));
    u.setQuery(QString());
    u.setFragment(QString());
    return u;
}

void SyncService::attachModel(DrawingModel *model)
{
    m_model = model;
}

void SyncService::connectWebSocket()
{
    if (!m_httpBase.isValid())
        return;
    const QUrl wsUrl = wsUrlFromHttp(m_httpBase);
    m_wsConnected = false;
    emit statusMessage(tr("Conectando WebSocket..."));
    m_ws->open(wsUrl);
}

void SyncService::fetchCanvas()
{
    if (!m_httpBase.isValid() || !m_model)
        return;
    QUrl u = m_httpBase;
    u.setPath(QStringLiteral("/canvas"));
    QNetworkRequest req(u);
    QNetworkReply *rep = m_nam->get(req);
    rep->setProperty("kind", QStringLiteral("get_canvas"));
}

void SyncService::saveCanvas()
{
    sendSaveRequest(false);
}

void SyncService::sendSaveRequest(bool isRetry)
{
    if (!m_httpBase.isValid() || !m_model)
        return;
    QUrl u = m_httpBase;
    u.setPath(QStringLiteral("/canvas/save"));

    const QJsonArray pendingArr = m_model->pendingStrokesJson();
    if (pendingArr.isEmpty() && !isRetry) {
        emit saveCompleted(true);
        return;
    }

    m_lastSaveStrokeIds.clear();
    for (const QJsonValue &v : pendingArr) {
        if (!v.isObject())
            continue;
        m_lastSaveStrokeIds.append(v.toObject().value(QStringLiteral("id")).toString());
    }

    QJsonObject root;
    root.insert(QStringLiteral("baseRevision"), m_model->serverRevision());
    root.insert(QStringLiteral("strokes"), pendingArr);

    QNetworkRequest req(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(kJsonContentType));
    const QByteArray body = QJsonDocument(root).toJson(QJsonDocument::Compact);

    QNetworkReply *rep = m_nam->post(req, body);
    rep->setProperty("kind", QStringLiteral("save_canvas"));
    rep->setProperty("retry", isRetry);
    if (isRetry)
        emit statusMessage(tr("Reintentando guardado..."));
}

void SyncService::publishLocalStroke(const Stroke &stroke)
{
    if (!m_ws || m_ws->state() != QAbstractSocket::ConnectedState)
        return;
    QJsonObject msg;
    msg.insert(QStringLiteral("type"), QStringLiteral("stroke"));
    msg.insert(QStringLiteral("stroke"), stroke.toJson());
    m_ws->sendTextMessage(QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact)));
}

void SyncService::onHttpFinished(QNetworkReply *reply)
{
    if (!reply)
        return;
    const QString kind = reply->property("kind").toString();
    const QByteArray body = reply->readAll();
    const bool retry = reply->property("retry").toBool();

    if (reply->error() != QNetworkReply::NoError) {
        const QString host = m_httpBase.isValid()
            ? m_httpBase.toString(QUrl::RemovePassword)
            : QStringLiteral("?");
        emit statusMessage(tr("Error de red: %1 | %2").arg(reply->errorString(), host));
        if (kind == QStringLiteral("save_canvas"))
            emit saveCompleted(false);
        reply->deleteLater();
        return;
    }

    if (kind == QStringLiteral("get_canvas")) {
        const QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            emit statusMessage(tr("Respuesta GET invalida"));
            reply->deleteLater();
            return;
        }
        handleCanvasGet(doc.object());
        reply->deleteLater();
        return;
    }
    if (kind == QStringLiteral("save_canvas")) {
        handleSaveResponse(body, retry);
        reply->deleteLater();
        return;
    }
    reply->deleteLater();
}

void SyncService::handleCanvasGet(const QJsonObject &obj)
{
    if (!m_model)
        return;
    const qint64 rev = qint64(obj.value(QStringLiteral("revision")).toDouble());
    const QJsonArray strokes = obj.value(QStringLiteral("strokes")).toArray();

    if (rev == m_model->serverRevision() && rev != 0
        && strokes.size() == m_model->strokes().size()) {
        return;
    }

    m_model->applyFullServerState(strokes, rev);
    emit statusMessage(tr("Lienzo cargado (revision %1)").arg(rev));
}

void SyncService::handleSaveResponse(const QByteArray &body, bool isRetry)
{
    if (!m_model)
        return;
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        emit statusMessage(tr("Respuesta POST invalida"));
        emit saveCompleted(false);
        return;
    }
    const QJsonObject o = doc.object();
    if (o.value(QStringLiteral("conflict")).toBool()) {
        const qint64 rev = qint64(o.value(QStringLiteral("revision")).toDouble());
        const QJsonArray strokes = o.value(QStringLiteral("strokes")).toArray();
        m_model->applyFullServerState(strokes, rev);
        emit statusMessage(tr("Conflicto de revision: fusionado desde servidor"));
        if (!isRetry && !m_model->pendingStrokes().isEmpty()) {
            sendSaveRequest(true);
            return;
        }
        emit saveCompleted(true);
        return;
    }
    if (o.value(QStringLiteral("ok")).toBool()) {
        const qint64 rev = qint64(o.value(QStringLiteral("revision")).toDouble());
        m_model->setServerRevision(rev);
        m_model->markStrokesSynced(m_lastSaveStrokeIds);
        emit statusMessage(tr("Guardado (revision %1)").arg(rev));
        emit saveCompleted(true);
        return;
    }
    emit statusMessage(tr("Guardado rechazado por el servidor"));
    emit saveCompleted(false);
}

void SyncService::onWsTextMessage(const QString &message)
{
    if (!m_model)
        return;
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject())
        return;
    const QJsonObject o = doc.object();
    const QString type = o.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("stroke")) {
        const QJsonObject so = o.value(QStringLiteral("stroke")).toObject();
        const Stroke s = Stroke::fromJson(so);
        m_model->ingestRemoteStroke(s);
        if (o.contains(QStringLiteral("revision")))
            m_model->setServerRevision(qint64(o.value(QStringLiteral("revision")).toDouble()));
        return;
    }
    if (type == QStringLiteral("full_resync")) {
        const qint64 rev = qint64(o.value(QStringLiteral("revision")).toDouble());
        const QJsonArray strokes = o.value(QStringLiteral("strokes")).toArray();
        m_model->applyFullServerState(strokes, rev);
    }
}

void SyncService::onWsConnected()
{
    m_wsConnected = true;
    emit statusMessage(tr("WebSocket conectado"));
}

void SyncService::onWsDisconnected()
{
    m_wsConnected = false;
    emit statusMessage(tr("WebSocket desconectado"));
}