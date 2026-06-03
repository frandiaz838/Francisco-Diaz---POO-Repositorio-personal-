#include "Cactus.h"

#include <QPainter>
#include <QRandomGenerator>

Cactus::Cactus(int velocidad, QWidget* parent) : Obstaculo(parent) {
    setVelocidad(velocidad);

    // Tamano con variacion aleatoria (cactus chicos y grandes).
    const int alto  = QRandomGenerator::global()->bounded(35, 65);
    const int ancho = QRandomGenerator::global()->bounded(16, 28);
    m_brazos = QRandomGenerator::global()->bounded(0, 3);   // 0..2 brazos
    resize(ancho, alto);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

QRect Cactus::cajaColision() const {
    // Hitbox levemente reducida para un juego mas justo.
    return geometry().adjusted(3, 3, -3, -1);
}

void Cactus::avanzar() {
    move(x() - m_velocidad, y());
}

void Cactus::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QColor("#4a7c2f"));        // verde cactus
    p.setPen(QColor("#33561f"));

    const int w = width();
    const int h = height();
    const int tronco = w / 3;

    // Tronco central.
    p.drawRoundedRect(QRect(w / 2 - tronco / 2, 0, tronco, h), 4, 4);

    // Brazos laterales (segun m_brazos).
    if (m_brazos >= 1) {
        // brazo izquierdo
        p.drawRoundedRect(QRect(0, h / 3, tronco, tronco), 3, 3);
        p.drawRoundedRect(QRect(0, h / 3, tronco / 2, h / 3), 3, 3);
    }
    if (m_brazos >= 2) {
        // brazo derecho
        p.drawRoundedRect(QRect(w - tronco, h / 2, tronco, tronco), 3, 3);
        p.drawRoundedRect(QRect(w - tronco / 2, h / 2 - h / 4, tronco / 2, h / 3), 3, 3);
    }
}
