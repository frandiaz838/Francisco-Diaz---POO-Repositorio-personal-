#include "Ventana.h"
#include "ApiClima.h"
#include "Clima.h"
#include "Logger.h"

#include <QApplication>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// Widget interno que pinta la imagen de fondo manteniendo relacion de aspecto.
class FondoWidget : public QWidget {
    QPixmap m_img;
public:
    explicit FondoWidget(const QPixmap& img, QWidget* parent = nullptr)
        : QWidget(parent), m_img(img) {}
protected:
    void paintEvent(QPaintEvent* ev) override {
        QWidget::paintEvent(ev);
        if (m_img.isNull()) return;
        QPainter p(this);
        p.setOpacity(0.22);
        QPixmap scaled = m_img.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
        p.drawPixmap((width() - scaled.width()) / 2,
                     (height() - scaled.height()) / 2, scaled);
    }
};

// Helpers de estilo
static QLabel* mkLabel(const QString& txt, int size, const QString& color, bool bold = false) {
    QLabel* l = new QLabel(txt);
    l->setWordWrap(true);
    l->setStyleSheet(QString("color: %1; font-size: %2px; %3 background: transparent;")
                         .arg(color).arg(size)
                         .arg(bold ? "font-weight: bold;" : ""));
    return l;
}

static QFrame* mkCard() {
    QFrame* f = new QFrame;
    f->setStyleSheet(
        "QFrame { background-color: rgba(255,255,255,9); "
        "border-radius: 14px; border: 1px solid rgba(255,255,255,18); }");
    return f;
}

static QPushButton* mkBoton(const QString& txt, const QString& bgRgba) {
    QPushButton* b = new QPushButton(txt);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: 1px solid rgba(255,255,255,40); "
        "border-radius: 8px; padding: 8px 18px; font-size: 13px; font-weight: bold; } "
        "QPushButton:hover { background: rgba(74,144,226,180); } "
        "QPushButton:pressed { background: rgba(74,144,226,220); }").arg(bgRgba));
    return b;
}

Ventana::Ventana(const QPixmap& imagen, const DatosClima& clima,
                 const ConfigApp& config, QWidget* parent)
    : QMainWindow(parent),
      m_imagen(imagen),
      m_config(config),
      m_datosClima(clima)
{
    setWindowTitle("Octavio Brachetti — Panel Principal");
    setStyleSheet("QMainWindow { background-color: #111122; }");

    // ApiClima propio para refrescar el clima desde esta pantalla
    m_api   = new ApiClima(m_config, this);
    m_clima = new Clima(this);
    m_clima->inicializar();

    connect(m_api,   &ApiClima::climaActualizado, m_clima, &Clima::actualizarDesdeApi);
    connect(m_api,   &ApiClima::climaActualizado, this,    &Ventana::onClimaActualizado);
    connect(m_api,   &ApiClima::errorRed,         this,    &Ventana::onErrorRed);

    inicializar();

    // Fondo + layout principal
    FondoWidget* fondo = new FondoWidget(m_imagen);
    fondo->setStyleSheet("background: transparent;");
    setCentralWidget(fondo);

    QVBoxLayout* root = new QVBoxLayout(fondo);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    construirHeader(root);

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("background: transparent;");
    m_stack->addWidget(construirDashboardClima()); // index 0
    m_stack->addWidget(construirCV());             // index 1
    root->addWidget(m_stack, 1);

    // Reloj
    m_timerReloj = new QTimer(this);
    m_timerReloj->setInterval(1000);
    connect(m_timerReloj, &QTimer::timeout, this, &Ventana::actualizarReloj);
    m_timerReloj->start();
    actualizarReloj();

    renderClima();
    showFullScreen();
}

void Ventana::inicializar() {
    Logger::instancia().registrar("Pantalla Ventana inicializada");
}

void Ventana::mostrarError(const QString& msg) {
    statusBar()->showMessage("Error: " + msg, 5000);
    Logger::instancia().registrar("Ventana error: " + msg);
}

// ── Header con reloj y botones ────────────────────────────────────────────────
void Ventana::construirHeader(QVBoxLayout* parentLay) {
    QFrame* bar = new QFrame;
    bar->setStyleSheet("QFrame { background: rgba(0,0,0,140); "
                        "border-bottom: 1px solid rgba(74,144,226,80); }");
    QHBoxLayout* h = new QHBoxLayout(bar);
    h->setContentsMargins(28, 12, 28, 12);
    h->setSpacing(16);

    QLabel* titulo = new QLabel("Panel Principal");
    titulo->setStyleSheet("color: white; font-size: 16px; font-weight: bold; "
                          "background: transparent;");
    h->addWidget(titulo);

    h->addStretch();

    m_lblHora = new QLabel;
    m_lblHora->setStyleSheet("color: #4A90E2; font-size: 15px; font-weight: bold; "
                             "background: transparent;");
    h->addWidget(m_lblHora);

    h->addSpacing(16);

    m_btnVista = mkBoton("Ver CV", "rgba(74,144,226,140)");
    connect(m_btnVista, &QPushButton::clicked, this, &Ventana::onToggleVista);
    h->addWidget(m_btnVista);

    QPushButton* btnSalir = mkBoton("Salir", "rgba(220,80,80,140)");
    connect(btnSalir, &QPushButton::clicked, this, &Ventana::salir);
    h->addWidget(btnSalir);

    parentLay->addWidget(bar);
}

// ── Dashboard de clima ────────────────────────────────────────────────────────
QWidget* Ventana::construirDashboardClima() {
    QWidget* page = new QWidget;
    page->setStyleSheet("background: transparent;");

    QVBoxLayout* lay = new QVBoxLayout(page);
    lay->setContentsMargins(80, 60, 80, 60);
    lay->setSpacing(20);
    lay->addStretch();

    QFrame* card = mkCard();
    card->setMaximumWidth(720);
    card->setStyleSheet(
        "QFrame { background-color: rgba(255,255,255,12); "
        "border-radius: 18px; border: 1px solid rgba(74,144,226,60); }");

    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setContentsMargins(48, 36, 48, 36);
    cl->setSpacing(14);

    QLabel* titulo = new QLabel("Clima actual");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("color: #4A90E2; font-size: 14px; font-weight: bold; "
                          "letter-spacing: 2px; background: transparent;");
    cl->addWidget(titulo);

    m_lblCiudad = new QLabel;
    m_lblCiudad->setAlignment(Qt::AlignCenter);
    m_lblCiudad->setStyleSheet("color: white; font-size: 26px; font-weight: bold; "
                                "background: transparent;");
    cl->addWidget(m_lblCiudad);

    m_lblTemp = new QLabel;
    m_lblTemp->setAlignment(Qt::AlignCenter);
    m_lblTemp->setStyleSheet("color: white; font-size: 96px; font-weight: bold; "
                              "background: transparent;");
    cl->addWidget(m_lblTemp);

    m_lblDesc = new QLabel;
    m_lblDesc->setAlignment(Qt::AlignCenter);
    m_lblDesc->setStyleSheet("color: #cccccc; font-size: 18px; "
                              "background: transparent;");
    cl->addWidget(m_lblDesc);

    m_lblActualizado = new QLabel;
    m_lblActualizado->setAlignment(Qt::AlignCenter);
    m_lblActualizado->setStyleSheet("color: #888888; font-size: 12px; "
                                     "background: transparent;");
    cl->addWidget(m_lblActualizado);

    m_lblEstado = new QLabel;
    m_lblEstado->setAlignment(Qt::AlignCenter);
    m_lblEstado->setStyleSheet("color: #ffaa55; font-size: 12px; "
                                "background: transparent;");
    cl->addWidget(m_lblEstado);

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_btnRefrescar = mkBoton("Refrescar", "rgba(74,144,226,140)");
    connect(m_btnRefrescar, &QPushButton::clicked, this, &Ventana::onRefrescarClima);
    btnRow->addWidget(m_btnRefrescar);
    btnRow->addStretch();
    cl->addLayout(btnRow);

    QHBoxLayout* center = new QHBoxLayout;
    center->addStretch();
    center->addWidget(card);
    center->addStretch();
    lay->addLayout(center);

    lay->addStretch();
    return page;
}

// ── Refresco visual del clima ─────────────────────────────────────────────────
void Ventana::renderClima() {
    if (m_datosClima.valido) {
        m_lblTemp->setText(QString::number(m_datosClima.temperatura, 'f', 1) + " °C");
        m_lblCiudad->setText(m_datosClima.ciudad);
        m_lblDesc->setText(m_datosClima.descripcion);
        m_lblEstado->clear();
    } else {
        m_lblTemp->setText("-- °C");
        m_lblCiudad->setText(m_datosClima.ciudad.isEmpty() ? "Sin datos" : m_datosClima.ciudad);
        m_lblDesc->setText(m_datosClima.descripcion.isEmpty()
                               ? "sin conexion"
                               : m_datosClima.descripcion);
        m_lblEstado->setText("Modo offline (datos simulados)");
    }
    m_lblActualizado->setText("Ultima actualizacion: "
        + QDateTime::currentDateTime().toString("HH:mm:ss"));
}

void Ventana::onRefrescarClima() {
    m_btnRefrescar->setEnabled(false);
    m_btnRefrescar->setText("Actualizando...");
    m_lblEstado->setText("Consultando OpenWeatherMap...");
    m_lblEstado->setStyleSheet("color: #4A90E2; font-size: 12px; background: transparent;");
    Logger::instancia().registrar("Ventana: refrescar clima solicitado");
    m_api->obtenerClima();
}

void Ventana::onClimaActualizado(DatosClima d) {
    m_datosClima = d;
    renderClima();
    m_btnRefrescar->setEnabled(true);
    m_btnRefrescar->setText("Refrescar");
}

void Ventana::onErrorRed(QString msg) {
    m_lblEstado->setStyleSheet("color: #ffaa55; font-size: 12px; background: transparent;");
    m_lblEstado->setText("Error de red: " + msg);
    m_btnRefrescar->setEnabled(true);
    m_btnRefrescar->setText("Refrescar");
    mostrarError(msg);
}

// ── Toggle entre dashboard y CV ───────────────────────────────────────────────
void Ventana::onToggleVista() {
    int next = (m_stack->currentIndex() == 0) ? 1 : 0;
    m_stack->setCurrentIndex(next);
    m_btnVista->setText(next == 0 ? "Ver CV" : "Ver Clima");
    Logger::instancia().registrar(QString("Ventana: vista cambiada a %1")
                                      .arg(next == 0 ? "Clima" : "CV"));
}

void Ventana::actualizarReloj() {
    if (m_lblHora)
        m_lblHora->setText(QDateTime::currentDateTime().toString("dddd dd/MM  HH:mm:ss"));
}

void Ventana::salir() {
    Logger::instancia().registrar("Ventana: salida solicitada por el usuario");
    qApp->quit();
}

// ── CV (vista alternativa) ────────────────────────────────────────────────────
QWidget* Ventana::construirCV() {
    QWidget* page = new QWidget;
    page->setStyleSheet("background: transparent;");

    QVBoxLayout* outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; } "
                          "QScrollBar:vertical { background: rgba(255,255,255,15); "
                          "width: 8px; border-radius: 4px; }");
    scroll->viewport()->setStyleSheet("background: transparent;");
    outer->addWidget(scroll);

    QWidget* contenedor = new QWidget;
    contenedor->setAutoFillBackground(false);
    contenedor->setStyleSheet("background: transparent;");
    scroll->setWidget(contenedor);

    QVBoxLayout* lay = new QVBoxLayout(contenedor);
    lay->setContentsMargins(60, 36, 60, 40);
    lay->setSpacing(18);

    // Header: iniciales + nombre
    QFrame* header = mkCard();
    QHBoxLayout* hh = new QHBoxLayout(header);
    hh->setContentsMargins(28, 28, 28, 28);
    hh->setSpacing(24);

    QLabel* foto = new QLabel("OB");
    foto->setFixedSize(110, 110);
    foto->setAlignment(Qt::AlignCenter);
    foto->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #4A90E2,stop:1 #7B68EE); border-radius: 55px; "
        "color: white; font-size: 34px; font-weight: bold;");

    QWidget* infoW = new QWidget;
    infoW->setStyleSheet("background: transparent;");
    QVBoxLayout* iv = new QVBoxLayout(infoW);
    iv->setContentsMargins(0,0,0,0);
    iv->setSpacing(5);
    iv->addWidget(mkLabel("Octavio Brachetti", 28, "white", true));
    iv->addWidget(mkLabel("Desarrollador de Software  |  Estudiante de Ingenieria en Software", 15, "#4A90E2"));
    iv->addWidget(mkLabel("Cordoba, Argentina  ·  UBP — Universidad Blas Pascal", 13, "#aaaaaa"));
    iv->addWidget(mkLabel("C++  ·  Qt6  ·  Python  ·  FastAPI  ·  MySQL  ·  Docker", 12, "#cccccc"));

    hh->addWidget(foto);
    hh->addWidget(infoW, 1);
    lay->addWidget(header);

    // Helper de seccion
    auto seccion = [&](const QString& titulo) -> QVBoxLayout* {
        QFrame* card = mkCard();
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(24, 18, 24, 18);
        cl->setSpacing(10);

        QLabel* t = new QLabel(titulo);
        t->setStyleSheet("color: #4A90E2; font-size: 15px; font-weight: bold; "
                         "background: transparent; border: none;");
        cl->addWidget(t);

        QFrame* sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("color: rgba(74,144,226,70);");
        cl->addWidget(sep);

        lay->addWidget(card);
        return cl;
    };

    QVBoxLayout* s1 = seccion("Sobre mi");
    s1->addWidget(mkLabel(
        "Estudiante de 3 anio de Ingenieria en Software en UBP (Cordoba, Argentina). "
        "Apasionado por el desarrollo de aplicaciones de escritorio con C++/Qt y el "
        "backend con Python/FastAPI. Tengo experiencia integrando interfaces modernas "
        "con servicios en la nube, bases de datos MySQL y despliegue con Docker en VPS.",
        13, "#dddddd"));

    QVBoxLayout* s2 = seccion("Educacion");
    struct EdItem { QString titulo, inst, periodo; };
    for (const auto& e : QList<EdItem>{
             {"Ingenieria en Software",    "UBP — Universidad Blas Pascal",  "2023 — Presente"},
             {"Educacion Primaria y Secundaria", "Academia Arguello",         "2009 — 2023"},
         }) {
        QWidget* w = new QWidget;
        w->setStyleSheet("background: transparent;");
        QHBoxLayout* wh = new QHBoxLayout(w);
        wh->setContentsMargins(0,0,0,0);
        wh->setSpacing(12);
        QWidget* dot = new QWidget;
        dot->setFixedSize(9,9);
        dot->setStyleSheet("background: #4A90E2; border-radius: 4px;");
        QVBoxLayout* wv = new QVBoxLayout;
        wv->setSpacing(2);
        wv->addWidget(mkLabel(e.titulo, 13, "white", true));
        wv->addWidget(mkLabel(e.inst + "   ·   " + e.periodo, 12, "#aaaaaa"));
        wh->addWidget(dot);
        wh->addLayout(wv);
        wh->addStretch();
        s2->addWidget(w);
    }

    QVBoxLayout* s3 = seccion("Proyectos Destacados");
    struct ProjItem { QString nombre, desc, stack; };
    for (const auto& p : QList<ProjItem>{
             {"Tablero Kanban Colaborativo",
              "Tablero Kanban con Qt y backend FastAPI en VPS. Columnas y tarjetas arrastrables, "
              "persistencia en MySQL, deploy con Docker.",
              "C++17  ·  Qt6  ·  FastAPI  ·  MySQL  ·  Docker"},
             {"Login con Clima",
              "Login seguro con bloqueo temporal, consumo de API meteorologica, "
              "descarga de imagenes con cache local y modo offline.",
              "C++17  ·  Qt6  ·  OpenWeatherMap API  ·  QNetworkAccessManager"},
             {"Lienzo Colaborativo",
              "Canvas compartido en red local. Trazos en tiempo real sincronizados "
              "entre multiples clientes via FastAPI WebSocket.",
              "C++17  ·  Qt6  ·  FastAPI  ·  WebSocket"},
         }) {
        QFrame* pf = new QFrame;
        pf->setStyleSheet(
            "QFrame { background: rgba(74,144,226,12); border-radius: 10px; "
            "border: 1px solid rgba(74,144,226,35); }");
        QVBoxLayout* pl = new QVBoxLayout(pf);
        pl->setContentsMargins(16,12,16,12);
        pl->setSpacing(4);
        pl->addWidget(mkLabel(p.nombre, 13, "white", true));
        pl->addWidget(mkLabel(p.desc,   12, "#bbbbbb"));
        pl->addWidget(mkLabel("Stack: " + p.stack, 11, "#4A90E2"));
        s3->addWidget(pf);
    }

    QVBoxLayout* s4 = seccion("Habilidades Tecnicas");
    QWidget* skillW = new QWidget;
    skillW->setStyleSheet("background: transparent;");
    QGridLayout* sg = new QGridLayout(skillW);
    sg->setContentsMargins(0,0,0,0);
    sg->setSpacing(8);
    QStringList skills = {"C++17","Qt6","Python","FastAPI","MySQL",
                          "Docker","Git","CMake","REST APIs","Linux"};
    for (int i = 0; i < skills.size(); ++i) {
        QLabel* sk = new QLabel(skills[i]);
        sk->setAlignment(Qt::AlignCenter);
        sk->setStyleSheet(
            "background: rgba(74,144,226,25); color: #4A90E2; "
            "border: 1px solid rgba(74,144,226,55); border-radius: 6px; "
            "padding: 6px 14px; font-size: 12px;");
        sg->addWidget(sk, i / 5, i % 5);
    }
    s4->addWidget(skillW);

    QVBoxLayout* s5 = seccion("Contacto");
    QWidget* contW = new QWidget;
    contW->setStyleSheet("background: transparent;");
    QHBoxLayout* ch = new QHBoxLayout(contW);
    ch->setContentsMargins(0,0,0,0);
    ch->setSpacing(0);
    for (const auto& item : QList<QPair<QString,QString>>{
             {"octaviobrachetti210@gmail.com", ""},
             {"UBP — Cordoba, Argentina", ""},
             {"github.com/octaviobrachetti", ""},
         }) {
        ch->addWidget(mkLabel(item.first, 13, "#dddddd"));
        ch->addSpacing(28);
    }
    ch->addStretch();
    s5->addWidget(contW);

    lay->addStretch();
    return page;
}
