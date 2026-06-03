#pragma once
#include <QString>

// Clase base abstracta para todas las pantallas de la aplicacion.
// Define el contrato polimorfico: cada pantalla derivada debe implementar
// los cinco metodos virtuales puros. El flujo de la aplicacion trabaja
// contra punteros/referencias a Pantalla, garantizando el uso de polimorfismo.
class Pantalla {
public:
    virtual ~Pantalla() = default;

    // Construye los widgets visuales de la pantalla.
    virtual void inicializarUI() = 0;

    // Conecta los signals y slots propios de la pantalla.
    virtual void conectarEventos() = 0;

    // Carga datos iniciales (config, archivos, recursos).
    virtual void cargarDatos() = 0;

    // Valida el estado actual antes de continuar (login valido, etc).
    virtual bool validarEstado() = 0;

    // Registra una accion en el log con el nombre de la pantalla.
    virtual void registrarEvento(const QString& descripcion) = 0;

    // Identificador legible de la pantalla para el log.
    virtual QString nombrePantalla() const = 0;
};
