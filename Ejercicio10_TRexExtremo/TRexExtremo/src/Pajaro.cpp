#include "Pajaro.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>

Pajaro::Pajaro(int velocidad, int alturaSuelo, QWidget* parent) : Obstaculo(parent) {
    setVelocidad(velocidad);
    resize(46, 34);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Tipo aleatorio -> altura de vuelo distinta.
    const int r = QRandomGenerator::global()->bounded(3);
    m_tipoVuelo = static_cast<Tipo>(r);

    int y = alturaSuelo - height();          // por defecto: rasante (al ras del suelo)
    switch (m_tipoVuelo) {
    case Tipo::Alto:    y = alturaSuelo - 150; break;   // hay que agacharse o no saltar
    case Tipo::Medio:   y = alturaSuelo - 95;  break;   // a la altura del salto
    case Tipo::Rasante: y = alturaSuelo - 42;  break;   // bajo: hay que saltar
    }
    move(parent ? parent->width() : 800, y);

    // QTimer PROPIO de este pajaro. Conexion en estilo viejo SIGNAL/SLOT
    // (pedido explicito de la consigna). Misma cadencia que el timer principal
    // para que se mueva parejo con los cactus.
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(mover()));
    m_timer->start(16);
}

Pajaro::~Pajaro() = default;

void Pajaro::detenerTimer() {
    if (m_timer)
        m_timer->stop();
}

QRect Pajaro::cajaColision() const {
    return geometry().adjusted(4, 6, -4, -6);
}

void Pajaro::avanzar() {
    move(x() - m_velocidad, y());
}

void Pajaro::mover() {
    avanzar();
    // Aleteo: alterna la posicion de las alas cada varios pasos.
    if (++m_contadorAleteo % 8 == 0) {
        m_alasArriba = !m_alasArriba;
        update();
    }
    if (fueraDePantalla()) {
        m_timer->stop();
        emit salioDePantalla(this);          // la escena lo elimina
    }
}

void Pajaro::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // Cuerpo.
    p.setBrush(QColor("#444"));
    p.setPen(QColor("#222"));
    p.drawEllipse(QRect(w / 4, h / 3, w / 2, h / 3));

    // Cabeza + pico.
    p.drawEllipse(QRect(w / 2, h / 4, w / 4, h / 4));
    p.setBrush(QColor("#e8a33d"));
    QPolygon pico;
    pico << QPoint(w - 4, h / 3) << QPoint(w * 3 / 4, h / 3) << QPoint(w * 3 / 4, h / 2);
    p.drawPolygon(pico);

    // Alas (aletean segun m_alasArriba).
    p.setBrush(QColor("#666"));
    p.setPen(QColor("#222"));
    if (m_alasArriba) {
        QPolygon ala;
        ala << QPoint(w / 3, h / 2) << QPoint(w / 2, 0) << QPoint(w * 2 / 3, h / 2);
        p.drawPolygon(ala);
    } else {
        QPolygon ala;
        ala << QPoint(w / 3, h / 2) << QPoint(w / 2, h) << QPoint(w * 2 / 3, h / 2);
        p.drawPolygon(ala);
    }
}
