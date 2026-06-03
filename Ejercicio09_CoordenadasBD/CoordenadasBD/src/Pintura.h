#pragma once
#include <QColor>
#include <QList>
#include <QWidget>
#include "Modelos.h"
#include "Pantalla.h"

// Lienzo de dibujo a mano alzada. Hereda de QWidget y de Pantalla.
// Controles:
//   - Mouse: dibujo a mano alzada (presionar + arrastrar).
//   - Rueda del mouse: ajusta el grosor del pincel.
//   - Teclas R / G / B: cambian el color del pincel.
//   - Escape: borra el lienzo completo.
//   - Ctrl+Z: deshace (hasta las ultimas 10 acciones).
class Pintura : public QWidget, public Pantalla {
    Q_OBJECT
public:
    static constexpr int kMaxUndo = 10;

    explicit Pintura(QWidget* parent = nullptr);
    ~Pintura() override = default;

    // ----- Implementaciones de Pantalla (virtuales puros) -----
    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "Pintura"; }

    // ----- API de persistencia / estado -----
    QList<Trazo> trazos() const { return m_trazos; }
    void         setTrazos(const QList<Trazo>& trazos);   // carga + repaint
    void         borrarTodo();                            // limpia lienzo e historial

    QColor color()  const { return m_color; }
    int    grosor() const { return m_grosor; }

signals:
    void colorCambiado(QColor color);
    void grosorCambiado(int grosor);
    void cantidadTrazosCambiada(int cantidad);

protected:
    void mousePressEvent(QMouseEvent* ev)   override;
    void mouseMoveEvent(QMouseEvent* ev)    override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev)        override;
    void keyPressEvent(QKeyEvent* ev)       override;
    void paintEvent(QPaintEvent* ev)        override;

private:
    void confirmarTrazoActual();
    void deshacer();
    void emitirEstado();

    QList<Trazo> m_trazos;     // trazos confirmados (lo que se dibuja)
    Trazo        m_actual;     // trazo en curso mientras se arrastra el mouse
    bool         m_dibujando = false;

    QColor m_color  = Qt::black;
    int    m_grosor = 3;

    // Cuenta de cuantos de los trazos finales pueden deshacerse (tope 10).
    int m_undosDisponibles = 0;
};
