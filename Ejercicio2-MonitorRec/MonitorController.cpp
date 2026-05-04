#include "MonitorController.h"
#include <QNetworkRequest>

MonitorController::MonitorController(QObject *parent)
    : QObject(parent), solicitudEnCurso(false) {
    manager = new QNetworkAccessManager(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MonitorController::solicitarDatos);
}

void MonitorController::configurar(QString url, int intervaloSegundos) {
    urlServidor = url.trimmed();
    if (urlServidor.isEmpty()) {
        timer->stop();
        return;
    }
    timer->start(intervaloSegundos * 1000);
}

void MonitorController::detener() {
    timer->stop();
}

void MonitorController::solicitarDatos() {
    if (urlServidor.isEmpty() || solicitudEnCurso)
        return;

    QUrl u(urlServidor);
    if (!u.isValid() || u.scheme().isEmpty()) {
        emit errorOcurrido("URL inválida o sin esquema (http/https).");
        return;
    }

    solicitudEnCurso = true;
    QNetworkRequest request(u);
    request.setHeader(QNetworkRequest::UserAgentHeader, "MonitorVPS/1.0");
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        alFinalizarRespuesta(reply);
    });
}

void MonitorController::alFinalizarRespuesta(QNetworkReply *reply) {
    solicitudEnCurso = false;

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOcurrido(reply->errorString());
        reply->deleteLater();
        return;
    }

    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (code >= 400) {
        emit errorOcurrido(QString("HTTP %1").arg(code));
        reply->deleteLater();
        return;
    }

    QJsonParseError parseErr;
    const QByteArray body = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseErr);

    if (doc.isNull() || !doc.isObject()) {
        emit errorOcurrido(QStringLiteral("Respuesta no es JSON: %1")
                               .arg(parseErr.errorString()));
        reply->deleteLater();
        return;
    }

    emit datosRecibidos(doc.object());
    reply->deleteLater();
}