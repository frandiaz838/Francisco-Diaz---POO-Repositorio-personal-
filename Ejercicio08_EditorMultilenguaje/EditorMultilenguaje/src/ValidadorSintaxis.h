#pragma once
#include <QString>

struct ResultadoValidacion {
    bool    valido = true;
    QString mensaje;        // diagnostico amigable al usuario
};

// Clase abstracta base para validadores de sintaxis por linea.
// Cada lenguaje implementa su propia version. El flujo de la aplicacion
// trabaja contra punteros ValidadorSintaxis*, demostrando polimorfismo.
class ValidadorSintaxis {
public:
    virtual ~ValidadorSintaxis() = default;

    // Valida una linea aislada del editor. Devuelve si es valida y un
    // mensaje de diagnostico legible para el usuario.
    virtual ResultadoValidacion validarLinea(const QString& linea) = 0;

    // Nombre del lenguaje (para UI y log).
    virtual QString nombreLenguaje() const = 0;

protected:
    // Helpers comunes a todos los validadores
    bool parentesisBalanceados(const QString& linea) const;
    bool comillasBalanceadas(const QString& linea) const;
    QString sinComentario(const QString& linea, const QString& marcaComentario) const;
};
