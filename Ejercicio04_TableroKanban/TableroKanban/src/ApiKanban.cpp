#include "ApiKanban.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QtCore/qoverload.h>

ApiKanban::ApiKanban(const QString& baseUrl,
                     const QString& basicUser,
                     const QString& basicPass,
                     QObject* parent)
    : QObject(parent), m_baseUrl(baseUrl)
{
    m_basicHeader = "Basic " + (basicUser + ":" + basicPass).toUtf8().toBase64();
}

QNetworkRequest ApiKanban::prepararRequest(const QString& path) const {
    QNetworkRequest req(QUrl(m_baseUrl + path));
    req.setRawHeader("Authorization", m_basicHeader);
    req.setRawHeader("Content-Type",  "application/json");
    req.setRawHeader("Accept",        "application/json");

    // El VPS usa cert autofirmado: ignoramos verificacion SSL
    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
    ssl.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(ssl);
    return req;
}

// Macro interna para conectar ignoreSslErrors y finished en una reply
static void conectarReply(QNetworkReply* reply, std::function<void()> onFinished) {
    QObject::connect(reply, &QNetworkReply::sslErrors, reply,
                     qOverload<const QList<QSslError>&>(&QNetworkReply::ignoreSslErrors));
    QObject::connect(reply, &QNetworkReply::finished, reply, [=]() {
        onFinished();
    });
}

// ── GET /kanban/board ─────────────────────────────────────────────────────────

void ApiKanban::fetchBoard() {
    QNetworkReply* reply = m_nam.get(prepararRequest("/api/kanban/board"));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("GET /board: " + reply->errorString());
            return;
        }
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isArray()) {
            emit errorApi("Respuesta invalida del servidor");
            return;
        }
        emit boardActualizado(doc.array());
    });
}

// ── Columnas ──────────────────────────────────────────────────────────────────

void ApiKanban::crearColumna(const QString& nombre) {
    QJsonObject body;
    body["name"] = nombre;
    QNetworkReply* reply = m_nam.post(
        prepararRequest("/api/kanban/columns"),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Crear columna: " + reply->errorString());
            return;
        }
        emit operacionOk("Columna creada");
    });
}

void ApiKanban::editarColumna(int id, const QString& nombre) {
    QJsonObject body;
    body["name"] = nombre;
    QNetworkReply* reply = m_nam.put(
        prepararRequest(QString("/api/kanban/columns/%1").arg(id)),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Editar columna: " + reply->errorString());
            return;
        }
        emit operacionOk("Columna editada");
    });
}

void ApiKanban::eliminarColumna(int id) {
    hacerDelete(QString("/api/kanban/columns/%1").arg(id));
}

// ── Tarjetas ──────────────────────────────────────────────────────────────────

void ApiKanban::crearTarjeta(int colId, const QString& titulo, const QString& desc) {
    QJsonObject body;
    body["column_id"]   = colId;
    body["title"]       = titulo;
    body["description"] = desc;
    QNetworkReply* reply = m_nam.post(
        prepararRequest("/api/kanban/cards"),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Crear tarjeta: " + reply->errorString());
            return;
        }
        emit operacionOk("Tarjeta creada");
    });
}

void ApiKanban::editarTarjeta(int id, const QString& titulo, const QString& desc) {
    QJsonObject body;
    body["title"]       = titulo;
    body["description"] = desc;
    QNetworkReply* reply = m_nam.put(
        prepararRequest(QString("/api/kanban/cards/%1").arg(id)),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Editar tarjeta: " + reply->errorString());
            return;
        }
        emit operacionOk("Tarjeta editada");
    });
}

void ApiKanban::eliminarTarjeta(int id) {
    hacerDelete(QString("/api/kanban/cards/%1").arg(id));
}

void ApiKanban::moverTarjeta(int cardId, int colId, int pos) {
    QJsonObject body;
    body["column_id"] = colId;
    body["position"]  = pos;
    QNetworkReply* reply = m_nam.put(
        prepararRequest(QString("/api/kanban/cards/%1/move").arg(cardId)),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Mover tarjeta: " + reply->errorString());
            return;
        }
        emit operacionOk("Tarjeta movida");
    });
}

void ApiKanban::hacerDelete(const QString& path) {
    QNetworkReply* reply = m_nam.deleteResource(prepararRequest(path));
    conectarReply(reply, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorApi("Eliminar: " + reply->errorString());
            return;
        }
        emit operacionOk("Eliminado");
    });
}
