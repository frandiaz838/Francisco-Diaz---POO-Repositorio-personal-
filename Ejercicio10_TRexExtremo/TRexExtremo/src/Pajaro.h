#pragma once
#include "Obstaculo.h"

class QTimer;

// Pajaro: obstaculo volador. A diferencia del cactus, CADA pajaro tiene su
// PROPIO QTimer que lo mueve a la izquierda de forma independiente.
// Hay distintos tipos, que determinan la altura a la que vuela.
class Pajaro : public Obstaculo {
    Q_OBJECT
public:
    enum class Tipo { Alto, Medio, Rasante };

    // velocidad: pixeles por paso del timer propio.
    // alturaSuelo: y del piso de la escena (para ubicar al pajaro segun tipo).
    explicit Pajaro(int velocidad, int alturaSuelo, QWidget* parent = nullptr);
    ~Pajaro() override;

    QRect   cajaColision() const override;
    void    avanzar()            override;   // usado por el timer propio
    QString tipo() const         override { return "pajaro"; }

    void detenerTimer();                     // se llama en Game Over

signals:
    void salioDePantalla(Pajaro* quien);     // para que la escena lo borre

private slots:
    void mover();                            // slot del QTimer propio

protected:
    void paintEvent(QPaintEvent* ev) override;

private:
    QTimer* m_timer    = nullptr;
    Tipo    m_tipoVuelo = Tipo::Medio;
    bool    m_alasArriba = false;            // animacion de aleteo
    int     m_contadorAleteo = 0;            // por instancia (no compartido)
};
