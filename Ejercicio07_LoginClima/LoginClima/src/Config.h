#pragma once
#include "Modelos.h"

class Config {
public:
    static Config& instancia();
    bool cargar(const QString& ruta);
    const ConfigApp& datos() const { return m_datos; }

private:
    Config() = default;
    ConfigApp m_datos;
};
