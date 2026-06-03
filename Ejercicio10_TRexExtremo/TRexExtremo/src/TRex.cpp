#include "TRex.h"

#include <QPainter>

TRex::TRex(int alturaSuelo, QWidget* parent)
    : QWidget(parent), m_alturaSuelo(alturaSuelo) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    resize(kAnchoNormal, kAltoNormal);
    reiniciar();
}

void TRex::reiniciar() {
    m_velY   = 0.0;
    m_estado = Estado::Corriendo;
    resize(kAnchoNormal, kAltoNormal);
    move(m_xInicial, m_alturaSuelo - height());
}

void TRex::apoyarEnSuelo() {
    move(x(), m_alturaSuelo - height());
}

// ── Acciones del jugador ───────────────────────────────────────────────
void TRex::saltar() {
    // Solo se puede saltar si esta en el suelo (no en el aire).
    if (m_estado == Estado::Saltando)
        return;
    m_estado = Estado::Saltando;
    // Si venia agachado, vuelve a tamano normal antes de saltar.
    resize(kAnchoNormal, kAltoNormal);
    m_velY = kImpulso;
}

void TRex::agacharse(bool activado) {
    if (m_estado == Estado::Saltando)
        return;   // en el aire no se agacha
    if (activado && m_estado != Estado::Agachado) {
        m_estado = Estado::Agachado;
        resize(kAnchoAgachado, kAltoAgachado);
        apoyarEnSuelo();
    } else if (!activado && m_estado == Estado::Agachado) {
        m_estado = Estado::Corriendo;
        resize(kAnchoNormal, kAltoNormal);
        apoyarEnSuelo();
    }
    update();
}

void TRex::adelantarse() {
    const int maxX = (parentWidget() ? parentWidget()->width() : 800) - width() - 20;
    move(qMin(x() + kPasoHorizontal, maxX), y());
}

void TRex::frenarse() {
    move(qMax(x() - kPasoHorizontal, 5), y());
}

// ── Fisica (la llama el timer principal del juego) ─────────────────────
void TRex::aplicarFisica() {
    if (m_estado == Estado::Saltando) {
        int nuevaY = y() + static_cast<int>(m_velY);
        m_velY += kGravedad;
        const int suelo = m_alturaSuelo - height();
        if (nuevaY >= suelo) {
            nuevaY = suelo;
            m_velY = 0.0;
            m_estado = Estado::Corriendo;
        }
        move(x(), nuevaY);
    }
    // Animacion de patas mientras corre.
    if (m_estado != Estado::Saltando)
        ++m_pasoCorrida;
    update();
}

QRect TRex::cajaColision() const {
    // Hitbox un poco mas chica que el sprite para que sea justo.
    return geometry().adjusted(6, 4, -6, -2);
}

// ── Dibujo ─────────────────────────────────────────────────────────────
void TRex::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QColor("#535353"));
    p.setPen(Qt::NoPen);

    const int w = width();
    const int h = height();

    if (m_estado == Estado::Agachado) {
        // Cuerpo alargado y bajo.
        p.drawRoundedRect(QRect(0, h / 4, w, h / 2), 6, 6);
        // Cabeza al frente.
        p.drawRoundedRect(QRect(w - 22, 0, 22, h * 3 / 5), 5, 5);
        // Ojo.
        p.setBrush(Qt::white);
        p.drawEllipse(QPoint(w - 8, h / 5), 3, 3);
        return;
    }

    // De pie (corriendo o saltando).
    // Cuerpo.
    p.drawRoundedRect(QRect(2, h / 3, w - 16, h / 2), 6, 6);
    // Cola.
    QPolygon cola;
    cola << QPoint(2, h / 3 + 4) << QPoint(-6, h / 3) << QPoint(2, h / 3 + 14);
    p.drawPolygon(cola);
    // Cuello + cabeza.
    p.drawRoundedRect(QRect(w - 22, 2, 20, 22), 5, 5);
    // Mandibula.
    p.drawRect(QRect(w - 16, 18, 16, 6));
    // Ojo.
    p.setBrush(Qt::white);
    p.drawEllipse(QPoint(w - 8, 10), 2, 2);
    p.setBrush(QColor("#535353"));

    // Patas: alternan para simular la corrida (no al saltar).
    const bool pasoA = (m_pasoCorrida / 5) % 2 == 0;
    const int baseY = h - 8;
    if (m_estado == Estado::Saltando) {
        p.drawRect(QRect(w / 2 - 10, baseY, 6, 8));
        p.drawRect(QRect(w / 2 + 2,  baseY, 6, 8));
    } else if (pasoA) {
        p.drawRect(QRect(w / 2 - 10, baseY, 6, 8));
        p.drawRect(QRect(w / 2 + 4,  baseY - 3, 6, 8));
    } else {
        p.drawRect(QRect(w / 2 - 10, baseY - 3, 6, 8));
        p.drawRect(QRect(w / 2 + 4,  baseY, 6, 8));
    }
}
