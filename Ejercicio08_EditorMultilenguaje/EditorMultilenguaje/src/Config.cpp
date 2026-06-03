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

    m_datos.usuario           = s.value("login/usuario", "admin").toString();
    m_datos.password          = s.value("login/password", "1234").toString();
    m_datos.maxIntentos       = s.value("login/max_intentos", 3).toInt();
    m_datos.segundosBloqueo   = s.value("login/segundos_bloqueo", 30).toInt();
    m_datos.lenguajePorDefecto = s.value("editor/lenguaje_por_defecto", "C++").toString();
    m_datos.rutaExportacion   = s.value("editor/ruta_exportacion", "./exportacion.jpg").toString();
    m_datos.nombreApp         = s.value("app/nombre", "Editor Multilenguaje POO").toString();
    m_datos.organizacion      = s.value("app/organizacion", "UBP-POO").toString();
    return true;
}
