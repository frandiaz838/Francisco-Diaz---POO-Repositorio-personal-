#include "Login.h"
#include "Logger.h"

#include <QCloseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

Login::Login(const ConfigApp& config, QWidget* parent)
    : QWidget(parent), m_config(config) {
    setWindowTitle("Inicio de Sesión");
    resize(520, 480);
    setStyleSheet("QWidget { background: #111122; color: #e4e4e7; }");

    inicializarUI();
    conectarEventos();
    cargarDatos();
}

void Login::inicializarUI() {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(60, 60, 60, 60);
    root->setSpacing(16);
    root->addStretch();

    m_lblTitulo = new QLabel("Editor Multilenguaje");
    m_lblTitulo->setAlignment(Qt::AlignCenter);
    m_lblTitulo->setStyleSheet(
        "color: white; font-size: 28px; font-weight: bold; background: transparent;");
    root->addWidget(m_lblTitulo);

    m_lblSubtitulo = new QLabel("Ingrese credenciales para continuar");
    m_lblSubtitulo->setAlignment(Qt::AlignCenter);
    m_lblSubtitulo->setStyleSheet(
        "color: #4A90E2; font-size: 13px; background: transparent;");
    root->addWidget(m_lblSubtitulo);
    root->addSpacing(20);

    auto estiloInput = QString(
        "QLineEdit { background: rgba(255,255,255,12); color: white; "
        "border: 1px solid rgba(74,144,226,80); border-radius: 8px; "
        "padding: 10px 14px; font-size: 14px; } "
        "QLineEdit:focus { border: 1px solid #4A90E2; }");

    m_inUsuario = new QLineEdit;
    m_inUsuario->setPlaceholderText("Usuario");
    m_inUsuario->setStyleSheet(estiloInput);
    root->addWidget(m_inUsuario);

    m_inPassword = new QLineEdit;
    m_inPassword->setPlaceholderText("Password");
    m_inPassword->setEchoMode(QLineEdit::Password);
    m_inPassword->setStyleSheet(estiloInput);
    root->addWidget(m_inPassword);

    m_btnIngresar = new QPushButton("Ingresar");
    m_btnIngresar->setCursor(Qt::PointingHandCursor);
    m_btnIngresar->setStyleSheet(
        "QPushButton { background: #4A90E2; color: white; border: none; "
        "border-radius: 8px; padding: 12px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background: #5BA0F2; } "
        "QPushButton:disabled { background: #555; }");
    root->addWidget(m_btnIngresar);

    m_lblEstado = new QLabel(" ");
    m_lblEstado->setAlignment(Qt::AlignCenter);
    m_lblEstado->setStyleSheet(
        "color: #ff6b6b; font-size: 12px; background: transparent;");
    root->addWidget(m_lblEstado);

    root->addStretch();
}

void Login::conectarEventos() {
    // Uso obligatorio de signals/slots
    connect(m_btnIngresar, &QPushButton::clicked, this, &Login::onIngresar);
    connect(m_inPassword,  &QLineEdit::returnPressed, this, &Login::onIngresar);
    connect(m_inUsuario,   &QLineEdit::returnPressed, this, &Login::onIngresar);
}

void Login::cargarDatos() {
    m_inUsuario->setText(m_config.usuario);
    m_inUsuario->selectAll();
    m_inUsuario->setFocus();
    registrarEvento("Pantalla Login cargada. Usuario por defecto: " + m_config.usuario);
}

bool Login::validarEstado() {
    return m_validado;
}

void Login::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

// ── Slot principal ─────────────────────────────────────────────────────
void Login::onIngresar() {
    const QString usuario  = m_inUsuario->text().trimmed();
    const QString password = m_inPassword->text();

    if (usuario.isEmpty() || password.isEmpty()) {
        mostrarError("Complete todos los campos");
        return;
    }

    if (usuario == m_config.usuario && password == m_config.password) {
        m_validado = true;
        m_btnIngresar->setEnabled(false);
        mostrarOk("Acceso autorizado. Cargando editor...");
        registrarEvento("Login exitoso: usuario=" + usuario);
        emit loginExitoso();
        return;
    }

    m_intentosFallidos++;
    registrarEvento(QString("Intento fallido %1/%2 con usuario=%3")
                        .arg(m_intentosFallidos)
                        .arg(m_config.maxIntentos)
                        .arg(usuario));

    if (m_intentosFallidos >= m_config.maxIntentos) {
        registrarEvento("Bloqueo temporal activado tras " +
                        QString::number(m_intentosFallidos) + " intentos fallidos");
        emit bloqueoSolicitado(m_config.segundosBloqueo);
        return;
    }

    mostrarError(QString("Credenciales inválidas. Intentos restantes: %1")
                     .arg(m_config.maxIntentos - m_intentosFallidos));
    m_inPassword->clear();
    m_inPassword->setFocus();
}

void Login::mostrarError(const QString& msg) {
    m_lblEstado->setStyleSheet(
        "color: #ff6b6b; font-size: 12px; background: transparent;");
    m_lblEstado->setText(msg);
}

void Login::mostrarOk(const QString& msg) {
    m_lblEstado->setStyleSheet(
        "color: #7EC8E3; font-size: 12px; background: transparent;");
    m_lblEstado->setText(msg);
}

// ── Eventos redefinidos (responsabilidad de Login) ─────────────────────
void Login::keyPressEvent(QKeyEvent* ev) {
    // En Login, Esc limpia los campos
    if (ev->key() == Qt::Key_Escape) {
        m_inUsuario->clear();
        m_inPassword->clear();
        m_inUsuario->setFocus();
        registrarEvento("Tecla Esc: campos limpiados");
        return;
    }
    if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
        onIngresar();
        return;
    }
    QWidget::keyPressEvent(ev);
}

void Login::mousePressEvent(QMouseEvent* ev) {
    // En Login, click sobre fondo quita foco de los inputs
    if (ev->button() == Qt::LeftButton) {
        m_inUsuario->clearFocus();
        m_inPassword->clearFocus();
        setFocus();
    }
    QWidget::mousePressEvent(ev);
}

void Login::resizeEvent(QResizeEvent* ev) {
    // En Login, escalamos el titulo segun ancho
    int base = qMax(18, ev->size().width() / 22);
    m_lblTitulo->setStyleSheet(QString(
        "color: white; font-size: %1px; font-weight: bold; background: transparent;")
                                   .arg(base));
    QWidget::resizeEvent(ev);
}

void Login::closeEvent(QCloseEvent* ev) {
    // En Login, salir sin haberse autenticado registra el evento
    if (!m_validado)
        registrarEvento("Cierre de Login sin autenticacion");
    QWidget::closeEvent(ev);
}

void Login::focusInEvent(QFocusEvent* ev) {
    registrarEvento("Ventana Login obtuvo foco");
    QWidget::focusInEvent(ev);
}

void Login::focusOutEvent(QFocusEvent* ev) {
    registrarEvento("Ventana Login perdio foco");
    QWidget::focusOutEvent(ev);
}
