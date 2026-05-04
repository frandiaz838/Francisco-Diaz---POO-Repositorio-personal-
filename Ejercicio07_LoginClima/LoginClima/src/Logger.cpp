#include "Logger.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>

Logger& Logger::instancia() {
    static Logger inst;
    return inst;
}

Logger::Logger() : m_ruta("app.log") {}

void Logger::setRuta(const QString& ruta) {
    m_ruta = ruta;
}

void Logger::registrar(const QString& descripcion) {
    QFile archivo(m_ruta);
    if (!archivo.open(QIODevice::Append | QIODevice::Text))
        return;
    QTextStream out(&archivo);
    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    out << "[" << ts << "] " << descripcion << "\n";
}
