#include "ApiClima.h"
#include "Logger.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QUrlQuery>

ApiClima::ApiClima(const ConfigApp& config, QObject* parent)
    : QObject(parent), m_config(config)
{
    // Patron de clase: un solo manager con la senal finished(QNetworkReply*)
    m_manager = new QNetworkAccessManager(this);
    m_manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

    connect(m_manager, SIGNAL(finished(QNetworkReply*)),
            this,      SLOT(slot_respuesta(QNetworkReply*)));

    if (m_config.proxyHabilitado && !m_config.proxyHost.isEmpty()) {
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(m_config.proxyHost);
        proxy.setPort(static_cast<quint16>(m_config.proxyPuerto));
        m_manager->setProxy(proxy);
        Logger::instancia().registrar("Proxy configurado: " + m_config.proxyHost);
    }
}

void ApiClima::obtenerClima() {
    if (m_config.apiKey.isEmpty() || m_config.apiKey == "TU_API_KEY_AQUI") {
        Logger::instancia().registrar("API key no configurada, usando datos simulados");
        DatosClima sim;
        sim.temperatura = 22.0;
        sim.descripcion = "parcialmente nublado";
        sim.ciudad      = "Cordoba";
        sim.valido      = false;
        emit climaActualizado(sim);
        emit errorRed("API key no configurada en config.ini");
        return;
    }

    if (m_config.ciudad.trimmed().isEmpty()) {
        emit errorRed("Ciudad vacia en config.ini (api/city)");
        return;
    }

    QUrl url("https://api.openweathermap.org/data/2.5/weather");
    QUrlQuery q;
    q.addQueryItem("q",     m_config.ciudad);
    q.addQueryItem("units", m_config.unidades);
    q.addQueryItem("appid", m_config.apiKey);
    q.addQueryItem("lang",  "es");
    url.setQuery(q);

    Logger::instancia().registrar("Consultando clima: " + url.toString());

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", "LoginClima-Qt6/1.0");

    QNetworkReply* reply = m_manager->get(request);
    // Etiquetamos el reply para que el slot unico sepa como procesarlo
    reply->setProperty("tipo", "clima");
}

void ApiClima::descargarImagen(const QString& url, const QString& rutaCache) {
    if (QFile::exists(rutaCache)) {
        QPixmap px;
        if (px.load(rutaCache)) {
            Logger::instancia().registrar("Imagen desde cache: " + rutaCache);
            emit imagenDescargada(px, rutaCache);
            return;
        }
    }

    Logger::instancia().registrar("Descargando imagen: " + url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "LoginClima-Qt6/1.0");

    QNetworkReply* reply = m_manager->get(request);
    reply->setProperty("tipo",      "imagen");
    reply->setProperty("rutaCache", rutaCache);
}

// Slot unico: el QNetworkAccessManager dispara finished(QNetworkReply*) y
// nosotros decidimos como procesar segun la propiedad "tipo" del reply.
void ApiClima::slot_respuesta(QNetworkReply* reply) {
    const QString tipo = reply->property("tipo").toString();
    if (tipo == "imagen")
        procesarImagen(reply, reply->property("rutaCache").toString());
    else
        procesarClima(reply);

    reply->deleteLater();
}

void ApiClima::procesarClima(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        // Leemos el body: OWM suele devolver JSON con "message" aun en errores
        QByteArray body = reply->readAll();
        QString detalle = reply->errorString();
        QJsonParseError pe;
        QJsonDocument d = QJsonDocument::fromJson(body, &pe);
        if (pe.error == QJsonParseError::NoError && d.isObject()) {
            QString msg = d.object().value("message").toString();
            if (!msg.isEmpty()) detalle = msg;
        }
        Logger::instancia().registrar("Error red clima: " + detalle);
        DatosClima sim;
        sim.temperatura = 18.0;
        sim.descripcion = "sin conexion (simulado)";
        sim.ciudad      = m_config.ciudad;
        sim.valido      = false;
        emit climaActualizado(sim);
        emit errorRed(detalle);
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        emit errorRed("Respuesta de clima invalida");
        return;
    }
    QJsonObject obj = doc.object();
    if (obj.contains("cod") && obj["cod"].toInt() != 200) {
        QString msg = obj["message"].toString("error desconocido");
        emit errorRed("API clima: " + msg);
        return;
    }
    DatosClima datos;
    datos.ciudad      = obj["name"].toString();
    auto main         = obj["main"].toObject();
    datos.temperatura = main["temp"].toDouble();
    auto weather      = obj["weather"].toArray();
    if (!weather.isEmpty())
        datos.descripcion = weather[0].toObject()["description"].toString();
    datos.valido = true;
    Logger::instancia().registrar(
        QString("Clima: %1 %2 C %3")
            .arg(datos.ciudad)
            .arg(datos.temperatura, 0, 'f', 1)
            .arg(datos.descripcion));
    emit climaActualizado(datos);
}

void ApiClima::procesarImagen(QNetworkReply* reply, const QString& rutaCache) {
    if (reply->error() != QNetworkReply::NoError) {
        Logger::instancia().registrar("Error descargando imagen: " + reply->errorString());
        emit errorRed("No se pudo descargar la imagen: " + reply->errorString());
        return;
    }
    QByteArray data = reply->readAll();
    QPixmap px;
    if (!px.loadFromData(data)) {
        emit errorRed("Imagen recibida no es valida");
        return;
    }
    QFile f(rutaCache);
    if (f.open(QIODevice::WriteOnly))
        f.write(data);
    Logger::instancia().registrar("Imagen guardada en cache: " + rutaCache);
    emit imagenDescargada(px, rutaCache);
}
