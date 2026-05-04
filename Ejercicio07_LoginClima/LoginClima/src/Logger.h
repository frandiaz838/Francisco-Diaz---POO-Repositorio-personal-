#pragma once
#include <QString>

class Logger {
public:
    static Logger& instancia();
    void setRuta(const QString& ruta);
    void registrar(const QString& descripcion);

private:
    Logger();
    QString m_ruta;
};
