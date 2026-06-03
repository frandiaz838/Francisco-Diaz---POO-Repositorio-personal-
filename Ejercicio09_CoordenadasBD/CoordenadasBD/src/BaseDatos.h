#pragma once
#include <QList>
#include <QString>
#include "Modelos.h"

// Gestor singleton de la base de datos SQLite.
// Responsabilidades:
//  - abrir/crear la base y su esquema (usuarios, trazos, puntos)
//  - validar credenciales contra hash SHA-256 (nunca texto plano)
//  - persistir y reconstruir las coordenadas del dibujo
class BaseDatos {
public:
    static BaseDatos& instancia();

    // Abre la conexion SQLite, crea las tablas si no existen y siembra
    // el usuario por defecto (admin/1234) si la tabla esta vacia.
    bool abrir(const QString& ruta);

    // Devuelve true si <usuario> existe y el hash de <password> coincide.
    bool validarUsuario(const QString& usuario, const QString& password);

    // Guarda un NUEVO dibujo (no pisa los anteriores) con nombre y fecha/hora.
    // Devuelve el id del dibujo creado, o -1 si falla.
    int guardarDibujo(const QList<Trazo>& trazos, const QString& nombre);

    // Lista los dibujos guardados (mas recientes primero) para elegir.
    QList<DibujoInfo> listarDibujos();

    // Reconstruye un dibujo por id, respetando el orden de trazos y puntos.
    QList<Trazo> cargarDibujo(int dibujoId);

    // Borra un dibujo y sus trazos/puntos de la base.
    bool eliminarDibujo(int dibujoId);

    QString ultimoError() const { return m_ultimoError; }

private:
    BaseDatos() = default;
    static QString hashPassword(const QString& password);
    bool crearEsquema();
    void sembrarUsuarioPorDefecto();

    QString m_ultimoError;
};
