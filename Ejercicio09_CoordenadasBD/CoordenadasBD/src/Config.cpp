#include "Config.h"
#include <QCoreApplication>
#include <QFile>
#include <QSettings>

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

    m_datos.nombreApp    = s.value("app/nombre", "Coordenadas en Base de Datos").toString();
    m_datos.organizacion = s.value("app/organizacion", "UBP-POO").toString();
    m_datos.usuario      = s.value("login/usuario", "admin").toString();
    m_datos.rutaDb       = s.value("db/ruta", "coordenadas.db").toString();
    return true;
}
