#pragma once
#include "Obstaculo.h"

// Cactus: obstaculo terrestre. NO tiene timer propio; lo mueve el timer
// principal del juego, que llama avanzar() en cada frame.
class Cactus : public Obstaculo {
    Q_OBJECT
public:
    explicit Cactus(int velocidad, QWidget* parent = nullptr);

    QRect   cajaColision() const override;
    void    avanzar()            override;
    QString tipo() const         override { return "cactus"; }

protected:
    void paintEvent(QPaintEvent* ev) override;

private:
    int m_brazos = 2;   // cantidad de brazos (variacion visual)
};
