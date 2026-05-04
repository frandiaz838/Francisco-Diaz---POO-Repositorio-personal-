#pragma once
#include <QString>

// Interfaz abstracta comun a todas las pantallas de la aplicacion.
class Pantalla {
public:
    virtual ~Pantalla() = default;
    virtual void    inicializar()                    = 0;
    virtual void    mostrarError(const QString& msg) = 0;
    virtual QString nombrePantalla() const           = 0;
};
