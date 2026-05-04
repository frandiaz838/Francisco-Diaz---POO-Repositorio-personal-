#include "Login.h"
#include "ui_Login.h"
#include "ApiClima.h"
#include "Clima.h"
#include "Logger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QKeyEvent>
#include <QPainter>
#include <QStandardPaths>

static constexpr char USUARIO_OK[]  = "admin";
static constexpr char PASSWORD_OK[] = "1234";
static constexpr int  MAX_INTENTOS  = 3;
static constexpr int  SEGS_BLOQUEO  = 30;

Login::Login(const ConfigApp& config, QWidget* parent)
    : QWidget(parent), m_ui(new Ui::Login), m_config(config)
{
    m_ui->setupUi(this);
    setWindowTitle("Inicio de Sesion");
    resize(900, 620);

    m_api   = new ApiClima(m_config, this);
    m_clima = new Clima(this);
    m_clima->inicializar();

    m_timerReloj   = new QTimer(this);
    m_timerBloqueo = new QTimer(this);
    m_timerReloj->setInterval(1000);
    m_timerBloqueo->setInterval(1000);

    // ----- Signals / Slots -----
    connect(m_ui->btnIngresar, &QPushButton::clicked,  this, &Login::intentarLogin);
    connect(m_timerReloj,   &QTimer::timeout, this, &Login::actualizarReloj);
    connect(m_timerBloqueo, &QTimer::timeout, this, &Login::tickBloqueo);

    connect(m_api,   &ApiClima::climaActualizado, m_clima, &Clima::actualizarDesdeApi);
    connect(m_clima, &Clima::datosActualizados,   this,    [this]() {
        onClimaActualizado(m_clima->datos());
    });
    connect(m_api, &ApiClima::errorRed, this, &Login::onErrorRed);

    // Cache compartido para ambas imagenes
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (cacheDir.isEmpty())
        cacheDir = QCoreApplication::applicationDirPath();
    cacheDir += "/cache";
    QDir().mkpath(cacheDir);

    const QString rutaFondo     = cacheDir + "/fondo_login.jpg";
    const QString rutaPrincipal = cacheDir + "/imagen_principal.jpg";

    connect(m_api, &ApiClima::imagenDescargada, this,
            [this, rutaFondo, rutaPrincipal](QPixmap px, QString ruta) {
        if (ruta == rutaFondo)
            onFondoDescargado(px, ruta);
        else if (ruta == rutaPrincipal)
            onImagenPrincipalDescargada(px, ruta);
    });

    inicializar();
    m_api->descargarImagen(m_config.urlFondoLogin, rutaFondo);
    m_api->obtenerClima();
    m_timerReloj->start();
    actualizarReloj();
}

Login::~Login() {
    delete m_ui;
}

void Login::inicializar() {
    Logger::instancia().registrar("Pantalla Login inicializada");
    m_ui->lblError->clear();
}

void Login::mostrarError(const QString& msg) {
    m_ui->lblError->setStyleSheet("color: #ff6b6b; font-size: 12px; background: transparent;");
    m_ui->lblError->setText(msg);
    Logger::instancia().registrar("Login error: " + msg);
}

void Login::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    if (m_fondoLogin.isNull()) return;
    QPainter p(this);
    QPixmap scaled = m_fondoLogin.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
    int x = (width()  - scaled.width())  / 2;
    int y = (height() - scaled.height()) / 2;
    p.drawPixmap(x, y, scaled);
}

void Login::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        intentarLogin();
    else
        QWidget::keyPressEvent(event);
}

void Login::actualizarReloj() {
    m_ui->lblHora->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
}

void Login::onClimaActualizado(DatosClima datos) {
    if (datos.valido)
        m_ui->lblClima->setText(
            QString("  %1 C  %2").arg(datos.temperatura, 0, 'f', 1).arg(datos.descripcion));
    else
        m_ui->lblClima->setText("  Sin conexion");
}

void Login::onFondoDescargado(QPixmap px, QString /*ruta*/) {
    m_fondoLogin = px;
    update();
    Logger::instancia().registrar("Fondo login cargado");
}

void Login::onImagenPrincipalDescargada(QPixmap px, QString /*ruta*/) {
    m_imagenPrincipal = px;
    Logger::instancia().registrar("Imagen principal descargada");
    if (m_loginValidado) {
        emit loginExitoso(m_imagenPrincipal, m_clima->datos());
        close();
    }
}

void Login::onErrorRed(QString msg) {
    // Solo mostrar errores de red que no sean de clima (la simulacion ya los cubre)
    if (!m_loginValidado)
        mostrarError("Red: " + msg);
}

void Login::intentarLogin() {
    if (m_bloqueado || m_loginValidado) return;

    const QString usuario  = m_ui->inputUsuario->text().trimmed();
    const QString password = m_ui->inputPassword->text();

    if (usuario.isEmpty() || password.isEmpty()) {
        mostrarError("Complete todos los campos");
        return;
    }

    if (usuario == USUARIO_OK && password == PASSWORD_OK) {
        m_loginValidado = true;
        m_ui->btnIngresar->setEnabled(false);
        m_ui->inputUsuario->setEnabled(false);
        m_ui->inputPassword->setEnabled(false);
        m_ui->lblError->setStyleSheet("color: #7EC8E3; font-size: 12px; background: transparent;");
        m_ui->lblError->setText("Cargando recursos...");
        Logger::instancia().registrar("Login exitoso: usuario=" + usuario);

        // Empezar descarga de imagen principal (o cargar desde cache)
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (cacheDir.isEmpty())
            cacheDir = QCoreApplication::applicationDirPath();
        cacheDir += "/cache";
        m_api->descargarImagen(m_config.urlImagenPrincipal, cacheDir + "/imagen_principal.jpg");
    } else {
        m_intentosFallidos++;
        Logger::instancia().registrar(
            QString("Login fallido (intento %1): usuario=%2").arg(m_intentosFallidos).arg(usuario));

        if (m_intentosFallidos >= MAX_INTENTOS) {
            m_bloqueado    = true;
            m_segsBloqueo  = SEGS_BLOQUEO;
            m_ui->btnIngresar->setEnabled(false);
            m_ui->inputUsuario->setEnabled(false);
            m_ui->inputPassword->setEnabled(false);
            mostrarError(QString("Bloqueado por %1 segundos").arg(m_segsBloqueo));
            m_timerBloqueo->start();
            Logger::instancia().registrar("Login bloqueado por 3 intentos fallidos");
        } else {
            mostrarError(
                QString("Credenciales invalidas. Intentos restantes: %1")
                    .arg(MAX_INTENTOS - m_intentosFallidos));
        }
    }
}

void Login::tickBloqueo() {
    m_segsBloqueo--;
    if (m_segsBloqueo <= 0) {
        m_timerBloqueo->stop();
        m_bloqueado          = false;
        m_intentosFallidos   = 0;
        m_ui->btnIngresar->setEnabled(true);
        m_ui->inputUsuario->setEnabled(true);
        m_ui->inputPassword->setEnabled(true);
        m_ui->inputPassword->clear();
        m_ui->lblError->clear();
        Logger::instancia().registrar("Login desbloqueado");
    } else {
        m_ui->lblError->setText(QString("Bloqueado por %1 segundos").arg(m_segsBloqueo));
    }
}
