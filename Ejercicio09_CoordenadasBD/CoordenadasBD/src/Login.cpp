#include "Login.h"
#include "ui_Login.h"

#include "BaseDatos.h"
#include "Logger.h"

#include <QKeyEvent>

Login::Login(const ConfigApp& config, QWidget* parent)
    : QWidget(parent), m_ui(new Ui::Login), m_config(config) {
    m_ui->setupUi(this);
    inicializarUI();
    conectarEventos();
    cargarDatos();
}

Login::~Login() {
    delete m_ui;
}

void Login::inicializarUI() {
    setWindowTitle("Inicio de Sesión");
    resize(820, 560);
    m_ui->lblEstado->clear();
}

void Login::conectarEventos() {
    // Uso obligatorio de signals/slots
    connect(m_ui->btnIngresar,   &QPushButton::clicked,     this, &Login::onIngresar);
    connect(m_ui->inputPassword, &QLineEdit::returnPressed, this, &Login::onIngresar);
    connect(m_ui->inputUsuario,  &QLineEdit::returnPressed, this, &Login::onIngresar);
}

void Login::cargarDatos() {
    m_ui->inputUsuario->setText(m_config.usuario);
    m_ui->inputUsuario->selectAll();
    m_ui->inputUsuario->setFocus();
    registrarEvento("Pantalla Login cargada. Usuario sugerido: " + m_config.usuario);
}

bool Login::validarEstado() {
    return m_validado;
}

void Login::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

void Login::onIngresar() {
    const QString usuario  = m_ui->inputUsuario->text().trimmed();
    const QString password = m_ui->inputPassword->text();

    if (usuario.isEmpty() || password.isEmpty()) {
        mostrarError("Complete usuario y contraseña");
        return;
    }

    // Validacion contra los usuarios almacenados en SQLite.
    if (BaseDatos::instancia().validarUsuario(usuario, password)) {
        m_validado = true;
        m_ui->btnIngresar->setEnabled(false);
        mostrarOk("Acceso autorizado. Abriendo lienzo...");
        // Log de acceso exitoso (sin la contraseña).
        registrarEvento("Acceso exitoso: usuario=" + usuario);
        emit loginExitoso();
        return;
    }

    // Log de intento fallido (sin la contraseña).
    registrarEvento("Intento fallido: usuario=" + usuario);
    mostrarError("Credenciales inválidas");
    m_ui->inputPassword->clear();
    m_ui->inputPassword->setFocus();
}

void Login::mostrarError(const QString& msg) {
    m_ui->lblEstado->setStyleSheet("color: #ff6b6b; font-size: 12px; background: transparent;");
    m_ui->lblEstado->setText(msg);
}

void Login::mostrarOk(const QString& msg) {
    m_ui->lblEstado->setStyleSheet("color: #7EE3A0; font-size: 12px; background: transparent;");
    m_ui->lblEstado->setText(msg);
}

void Login::keyPressEvent(QKeyEvent* ev) {
    // En Login, Esc limpia los campos.
    if (ev->key() == Qt::Key_Escape) {
        m_ui->inputUsuario->clear();
        m_ui->inputPassword->clear();
        m_ui->inputUsuario->setFocus();
        registrarEvento("Tecla Esc: campos limpiados");
        return;
    }
    QWidget::keyPressEvent(ev);
}
