#pragma once
#include <QMainWindow>
#include "Modelos.h"
#include "Pantalla.h"

class Pintura;
class QLabel;

// Ventana principal post-login. Hereda de QMainWindow y de Pantalla.
// Hostea el lienzo Pintura, una barra de herramientas (Guardar / Cargar /
// Limpiar) y una barra de estado con color, grosor y la ayuda de teclas.
class Ventana : public QMainWindow, public Pantalla {
    Q_OBJECT
public:
    explicit Ventana(const ConfigApp& config, QWidget* parent = nullptr);
    ~Ventana() override = default;

    // ----- Implementaciones de Pantalla (virtuales puros) -----
    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "Ventana"; }

private slots:
    void onGuardar();
    void onCargar();
    void onLimpiar();
    void onColorCambiado(QColor color);
    void onGrosorCambiado(int grosor);
    void onCantidadTrazosCambiada(int cantidad);

private:
    ConfigApp m_config;
    Pintura*  m_pintura     = nullptr;
    QLabel*   m_lblColor    = nullptr;
    QLabel*   m_lblGrosor   = nullptr;
    QLabel*   m_lblTrazos   = nullptr;
    QLabel*   m_lblAyuda    = nullptr;
};
