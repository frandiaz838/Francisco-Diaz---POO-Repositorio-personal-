#include "Clima.h"
#include "Logger.h"

Clima::Clima(QObject* parent) : QObject(parent) {}

void Clima::inicializar() {
    Logger::instancia().registrar("Clima: inicializado");
}

void Clima::mostrarError(const QString& msg) {
    Logger::instancia().registrar("Clima error: " + msg);
    emit hayError(msg);
}

void Clima::actualizarDesdeApi(DatosClima datos) {
    m_datos = datos;
    Logger::instancia().registrar("Clima: datos actualizados");
    emit datosActualizados();
}
