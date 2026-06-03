#include "Pintura.h"
#include "Logger.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>

Pintura::Pintura(QWidget* parent) : QWidget(parent) {
    inicializarUI();
    conectarEventos();
    cargarDatos();
}

void Pintura::inicializarUI() {
    setMinimumSize(640, 420);
    setAutoFillBackground(true);
    // Fondo blanco para dibujar.
    setStyleSheet("background-color: white;");
    setCursor(Qt::CrossCursor);
    // StrongFocus para recibir eventos de teclado (R/G/B, Esc, Ctrl+Z).
    setFocusPolicy(Qt::StrongFocus);
}

void Pintura::conectarEventos() {
    // No hay signals/slots internos: el widget responde a eventos de mouse/teclado.
}

void Pintura::cargarDatos() {
    registrarEvento("Lienzo listo. Color negro, grosor 3.");
    emitirEstado();
}

bool Pintura::validarEstado() {
    return true;
}

void Pintura::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

void Pintura::emitirEstado() {
    emit colorCambiado(m_color);
    emit grosorCambiado(m_grosor);
    emit cantidadTrazosCambiada(m_trazos.size());
}

// ── Persistencia / estado ──────────────────────────────────────────────
void Pintura::setTrazos(const QList<Trazo>& trazos) {
    m_trazos = trazos;
    m_actual = Trazo();
    m_dibujando = false;
    // Tras una carga desde la base, no se puede deshacer lo cargado.
    m_undosDisponibles = 0;
    registrarEvento(QString("Trazos reconstruidos desde la base: %1").arg(m_trazos.size()));
    emitirEstado();
    update();
}

void Pintura::borrarTodo() {
    m_trazos.clear();
    m_actual = Trazo();
    m_dibujando = false;
    m_undosDisponibles = 0;
    emit cantidadTrazosCambiada(0);
    update();
}

// ── Eventos de mouse ───────────────────────────────────────────────────
void Pintura::mousePressEvent(QMouseEvent* ev) {
    if (ev->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(ev);
        return;
    }
    m_dibujando = true;
    m_actual = Trazo();
    m_actual.color  = m_color.name();
    m_actual.grosor = m_grosor;
    m_actual.puntos.append({ static_cast<double>(ev->position().x()),
                             static_cast<double>(ev->position().y()) });
    update();
}

void Pintura::mouseMoveEvent(QMouseEvent* ev) {
    if (!m_dibujando)
        return;
    m_actual.puntos.append({ static_cast<double>(ev->position().x()),
                             static_cast<double>(ev->position().y()) });
    update();
}

void Pintura::mouseReleaseEvent(QMouseEvent* ev) {
    if (ev->button() != Qt::LeftButton || !m_dibujando) {
        QWidget::mouseReleaseEvent(ev);
        return;
    }
    m_dibujando = false;
    confirmarTrazoActual();
}

void Pintura::confirmarTrazoActual() {
    // Ignoramos clicks sin movimiento (un solo punto se dibuja como punto).
    if (m_actual.puntos.isEmpty())
        return;

    m_trazos.append(m_actual);
    m_actual = Trazo();

    // Historial de undo acotado a las ultimas 10 acciones.
    if (m_undosDisponibles < kMaxUndo)
        ++m_undosDisponibles;

    registrarEvento(QString("Trazo confirmado (color=%1, grosor=%2). Total=%3")
                        .arg(m_color.name()).arg(m_grosor).arg(m_trazos.size()));
    emit cantidadTrazosCambiada(m_trazos.size());
    update();
}

// ── Rueda del mouse: grosor del pincel ─────────────────────────────────
void Pintura::wheelEvent(QWheelEvent* ev) {
    const int pasos = ev->angleDelta().y() / 120;   // 1 muesca = 120
    if (pasos != 0) {
        m_grosor = qBound(1, m_grosor + pasos, 40);
        emit grosorCambiado(m_grosor);
        registrarEvento(QString("Grosor del pincel: %1").arg(m_grosor));
        ev->accept();
        return;
    }
    QWidget::wheelEvent(ev);
}

// ── Teclado: R/G/B, Escape, Ctrl+Z ─────────────────────────────────────
void Pintura::keyPressEvent(QKeyEvent* ev) {
    if (ev->matches(QKeySequence::Undo)) {   // Ctrl+Z
        deshacer();
        return;
    }

    switch (ev->key()) {
    case Qt::Key_N:
        m_color = Qt::black;
        emit colorCambiado(m_color);
        registrarEvento("Color del pincel: negro");
        return;
    case Qt::Key_R:
        m_color = Qt::red;
        emit colorCambiado(m_color);
        registrarEvento("Color del pincel: rojo");
        return;
    case Qt::Key_G:
        m_color = Qt::green;
        emit colorCambiado(m_color);
        registrarEvento("Color del pincel: verde");
        return;
    case Qt::Key_B:
        m_color = Qt::blue;
        emit colorCambiado(m_color);
        registrarEvento("Color del pincel: azul");
        return;
    case Qt::Key_Escape:
        borrarTodo();
        registrarEvento("Lienzo borrado completo (Escape)");
        return;
    default:
        QWidget::keyPressEvent(ev);
    }
}

void Pintura::deshacer() {
    if (m_trazos.isEmpty() || m_undosDisponibles <= 0) {
        registrarEvento("Deshacer: no hay acciones disponibles (limite 10)");
        return;
    }
    m_trazos.removeLast();
    --m_undosDisponibles;
    registrarEvento(QString("Deshacer. Total=%1").arg(m_trazos.size()));
    emit cantidadTrazosCambiada(m_trazos.size());
    update();
}

// ── Dibujado ───────────────────────────────────────────────────────────
void Pintura::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::white);

    auto dibujarTrazo = [&p](const Trazo& t) {
        if (t.puntos.isEmpty())
            return;
        QPen pen(QColor(t.color), t.grosor, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);

        if (t.puntos.size() == 1) {
            // Click sin arrastre: un punto.
            p.drawPoint(QPointF(t.puntos.first().x, t.puntos.first().y));
            return;
        }
        QPainterPath path(QPointF(t.puntos.first().x, t.puntos.first().y));
        for (int i = 1; i < t.puntos.size(); ++i)
            path.lineTo(QPointF(t.puntos.at(i).x, t.puntos.at(i).y));
        p.drawPath(path);
    };

    for (const Trazo& t : m_trazos)
        dibujarTrazo(t);

    // Trazo en curso (mientras se arrastra el mouse).
    if (m_dibujando)
        dibujarTrazo(m_actual);
}
