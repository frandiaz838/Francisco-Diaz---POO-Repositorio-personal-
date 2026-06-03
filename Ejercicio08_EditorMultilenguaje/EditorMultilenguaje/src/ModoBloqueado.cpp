#include "ModoBloqueado.h"
#include "Logger.h"

#include <QCloseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

ModoBloqueado::ModoBloqueado(int segundos, QWidget* parent)
    : QWidget(parent), m_segundosRestantes(segundos) {
    setWindowTitle("Acceso Bloqueado");
    resize(520, 480);
    setStyleSheet("QWidget { background: #1a0a0a; color: #ff8080; }");

    inicializarUI();
    conectarEventos();
    cargarDatos();
}

void ModoBloqueado::inicializarUI() {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(60, 60, 60, 60);
    root->setSpacing(18);
    root->addStretch();

    m_lblTitulo = new QLabel("ACCESO BLOQUEADO");
    m_lblTitulo->setAlignment(Qt::AlignCenter);
    m_lblTitulo->setStyleSheet(
        "color: #ff5555; font-size: 28px; font-weight: bold; background: transparent; "
        "letter-spacing: 3px;");
    root->addWidget(m_lblTitulo);

    m_lblMensaje = new QLabel(
        "Demasiados intentos fallidos.\nEspere a que termine el bloqueo temporal.");
    m_lblMensaje->setAlignment(Qt::AlignCenter);
    m_lblMensaje->setStyleSheet(
        "color: #ffaaaa; font-size: 13px; background: transparent;");
    root->addWidget(m_lblMensaje);

    root->addSpacing(20);

    m_lblCountdown = new QLabel("--");
    m_lblCountdown->setAlignment(Qt::AlignCenter);
    m_lblCountdown->setStyleSheet(
        "color: white; font-size: 84px; font-weight: bold; background: transparent;");
    root->addWidget(m_lblCountdown);

    root->addStretch();
}

void ModoBloqueado::conectarEventos() {
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &ModoBloqueado::onTick);
}

void ModoBloqueado::cargarDatos() {
    m_lblCountdown->setText(QString::number(m_segundosRestantes));
    m_timer->start();
    registrarEvento(QString("Bloqueo iniciado: %1 segundos").arg(m_segundosRestantes));
}

bool ModoBloqueado::validarEstado() {
    return m_segundosRestantes <= 0;
}

void ModoBloqueado::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

void ModoBloqueado::onTick() {
    m_segundosRestantes--;
    m_lblCountdown->setText(QString::number(m_segundosRestantes));
    if (m_segundosRestantes <= 0) {
        m_timer->stop();
        registrarEvento("Bloqueo finalizado, regresando a Login");
        emit desbloqueado();
    }
}

// ── Eventos redefinidos (responsabilidad de ModoBloqueado) ─────────────
void ModoBloqueado::keyPressEvent(QKeyEvent* ev) {
    // En ModoBloqueado las teclas estan deshabilitadas
    registrarEvento(QString("Tecla ignorada durante bloqueo (codigo %1)").arg(ev->key()));
    ev->accept();
}

void ModoBloqueado::mousePressEvent(QMouseEvent* ev) {
    // En ModoBloqueado los clicks tambien se ignoran
    registrarEvento("Click ignorado durante bloqueo");
    ev->accept();
}

void ModoBloqueado::resizeEvent(QResizeEvent* ev) {
    int base = qMax(48, ev->size().width() / 10);
    m_lblCountdown->setStyleSheet(QString(
        "color: white; font-size: %1px; font-weight: bold; background: transparent;")
                                      .arg(base));
    QWidget::resizeEvent(ev);
}

void ModoBloqueado::closeEvent(QCloseEvent* ev) {
    // En ModoBloqueado no se puede cerrar la ventana hasta que termine el contador
    if (m_segundosRestantes > 0) {
        registrarEvento("Intento de cierre durante bloqueo: rechazado");
        ev->ignore();
        return;
    }
    QWidget::closeEvent(ev);
}

void ModoBloqueado::focusInEvent(QFocusEvent* ev) {
    registrarEvento("Foco en pantalla de bloqueo");
    QWidget::focusInEvent(ev);
}

void ModoBloqueado::focusOutEvent(QFocusEvent* ev) {
    registrarEvento("Pantalla de bloqueo perdio foco");
    QWidget::focusOutEvent(ev);
}
