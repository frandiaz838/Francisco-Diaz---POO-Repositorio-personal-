#pragma once
#include <QObject>
#include "Modelos.h"
#include "Pantalla.h"

// Clase derivada de Pantalla que gestiona el estado del clima.
class Clima : public QObject, public Pantalla {
    Q_OBJECT
public:
    explicit Clima(QObject* parent = nullptr);

    void    inicializar()                    override;
    void    mostrarError(const QString& msg) override;
    QString nombrePantalla() const           override { return "Clima"; }

    const DatosClima& datos() const { return m_datos; }

public slots:
    void actualizarDesdeApi(DatosClima datos);

signals:
    void datosActualizados();
    void hayError(QString msg);

private:
    DatosClima m_datos;
};
