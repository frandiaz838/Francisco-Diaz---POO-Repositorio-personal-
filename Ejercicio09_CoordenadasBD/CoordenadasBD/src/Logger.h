#pragma once
#include <QString>

// Logger singleton. Escribe accesos y eventos a un archivo de texto.
// IMPORTANTE: nunca debe recibir ni registrar contrasenas.
class Logger {
public:
    static Logger& instancia();
    void setRuta(const QString& ruta);
    void registrar(const QString& descripcion);

private:
    Logger();
    QString m_ruta;
};
