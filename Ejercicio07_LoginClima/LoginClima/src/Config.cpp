#include "Config.h"
#include <QSettings>
#include <QFile>
#include <QCoreApplication>

Config& Config::instancia() {
    static Config inst;
    return inst;
}

bool Config::cargar(const QString& ruta) {
    QString efectiva = ruta;
    if (!QFile::exists(efectiva))
        efectiva = QCoreApplication::applicationDirPath() + "/" + ruta;
    if (!QFile::exists(efectiva))
        return false;

    QSettings s(efectiva, QSettings::IniFormat);

    // QSettings INI trata la coma como separador de lista, asi que leemos
    // city como QStringList y volvemos a unir con coma para soportar
    // valores tipo "Cordoba,AR" (con o sin comillas en el ini).
    m_datos.apiKey   = s.value("api/key").toString();
    QStringList ciudadPartes = s.value("api/city").toStringList();
    m_datos.ciudad   = ciudadPartes.isEmpty() ? QStringLiteral("Cordoba,AR")
                                              : ciudadPartes.join(',');
    m_datos.unidades = s.value("api/units", "metric").toString();
    m_datos.urlFondoLogin      = s.value("imagenes/url_fondo_login").toString();
    m_datos.urlImagenPrincipal = s.value("imagenes/url_imagen_principal").toString();
    m_datos.proxyHabilitado    = s.value("proxy/habilitado", false).toBool();
    m_datos.proxyHost          = s.value("proxy/host").toString();
    m_datos.proxyPuerto        = s.value("proxy/puerto", 8080).toInt();
    return true;
}
