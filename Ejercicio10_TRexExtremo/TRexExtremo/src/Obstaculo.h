#pragma once
#include <QWidget>

// Clase base abstracta de todos los obstaculos del juego (cactus y pajaros).
// Hereda de QWidget: cada obstaculo es un widget hijo de la escena que se
// mueve con move(). El juego trabaja con punteros Obstaculo* y detecta las
// colisiones de forma polimorfica usando cajaColision().
class Obstaculo : public QWidget {
    Q_OBJECT
public:
    explicit Obstaculo(QWidget* parent = nullptr) : QWidget(parent) {}
    ~Obstaculo() override = default;

    // ----- Contrato polimorfico (virtuales puros) -----
    // Rectangulo de colision en coordenadas de la escena (padre).
    virtual QRect cajaColision() const = 0;
    // Mueve el obstaculo un paso a la izquierda segun su velocidad.
    virtual void avanzar() = 0;
    // Identificador del tipo ("cactus", "pajaro").
    virtual QString tipo() const = 0;

    // ----- Comportamiento comun -----
    void setVelocidad(int v) { m_velocidad = v; }
    int  velocidad() const   { return m_velocidad; }

    // True si el obstaculo salio por el borde izquierdo de la escena.
    bool fueraDePantalla() const { return x() + width() < 0; }

protected:
    int m_velocidad = 6;   // pixeles por paso
};
