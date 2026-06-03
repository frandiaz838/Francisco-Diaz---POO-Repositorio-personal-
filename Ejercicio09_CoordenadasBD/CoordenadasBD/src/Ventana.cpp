#include "Ventana.h"

#include "BaseDatos.h"
#include "Logger.h"
#include "Pintura.h"

#include <QDateTime>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

Ventana::Ventana(const ConfigApp& config, QWidget* parent)
    : QMainWindow(parent), m_config(config) {
    inicializarUI();
    conectarEventos();
    cargarDatos();
}

void Ventana::inicializarUI() {
    setWindowTitle(m_config.nombreApp);
    resize(960, 640);

    m_pintura = new Pintura(this);
    setCentralWidget(m_pintura);

    // Barra de herramientas con las acciones de persistencia.
    QToolBar* barra = addToolBar("Acciones");
    barra->setMovable(false);
    QAction* actGuardar = barra->addAction("Guardar");
    QAction* actCargar  = barra->addAction("Cargar");
    barra->addSeparator();
    QAction* actLimpiar = barra->addAction("Limpiar lienzo");

    connect(actGuardar, &QAction::triggered, this, &Ventana::onGuardar);
    connect(actCargar,  &QAction::triggered, this, &Ventana::onCargar);
    connect(actLimpiar, &QAction::triggered, this, &Ventana::onLimpiar);

    // Barra de estado: color, grosor, cantidad de trazos y ayuda de teclas.
    m_lblColor  = new QLabel;
    m_lblGrosor = new QLabel;
    m_lblTrazos = new QLabel;
    m_lblAyuda  = new QLabel("Mouse: dibujar  |  Rueda: grosor  |  R/G/B/N: color (N=negro)  |  Esc: borrar  |  Ctrl+Z: deshacer");
    statusBar()->addWidget(m_lblColor);
    statusBar()->addWidget(m_lblGrosor);
    statusBar()->addWidget(m_lblTrazos);
    statusBar()->addPermanentWidget(m_lblAyuda);
}

void Ventana::conectarEventos() {
    connect(m_pintura, &Pintura::colorCambiado,           this, &Ventana::onColorCambiado);
    connect(m_pintura, &Pintura::grosorCambiado,          this, &Ventana::onGrosorCambiado);
    connect(m_pintura, &Pintura::cantidadTrazosCambiada,  this, &Ventana::onCantidadTrazosCambiada);
}

void Ventana::cargarDatos() {
    // Refleja el estado inicial del lienzo en la barra de estado.
    onColorCambiado(m_pintura->color());
    onGrosorCambiado(m_pintura->grosor());
    onCantidadTrazosCambiada(m_pintura->trazos().size());
    m_pintura->setFocus();
    registrarEvento("Ventana principal abierta");
}

bool Ventana::validarEstado() {
    return true;
}

void Ventana::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

// ── Slots de persistencia ──────────────────────────────────────────────
void Ventana::onGuardar() {
    if (m_pintura->trazos().isEmpty()) {
        QMessageBox::information(this, "Guardar", "El lienzo está vacío: no hay nada para guardar.");
        return;
    }

    // Nombre del dibujo (con fecha/hora sugerida por defecto).
    const QString sugerido = "Dibujo " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    bool ok = false;
    QString nombre = QInputDialog::getText(this, "Guardar dibujo",
                                           "Nombre del dibujo:", QLineEdit::Normal,
                                           sugerido, &ok);
    if (!ok)
        return;
    nombre = nombre.trimmed();
    if (nombre.isEmpty())
        nombre = sugerido;

    const int id = BaseDatos::instancia().guardarDibujo(m_pintura->trazos(), nombre);
    if (id >= 0) {
        registrarEvento("Dibujo guardado: '" + nombre + "'");
        statusBar()->showMessage("Dibujo guardado: " + nombre, 3000);
    } else {
        const QString err = BaseDatos::instancia().ultimoError();
        registrarEvento("Error al guardar: " + err);
        QMessageBox::warning(this, "Guardar", "No se pudo guardar el dibujo:\n" + err);
    }
}

void Ventana::onCargar() {
    const QList<DibujoInfo> dibujos = BaseDatos::instancia().listarDibujos();
    if (dibujos.isEmpty()) {
        QMessageBox::information(this, "Cargar", "Todavía no hay dibujos guardados.");
        return;
    }

    // Lista para elegir: "nombre  —  fecha hora".
    QStringList items;
    for (const DibujoInfo& d : dibujos)
        items << QString("%1  —  %2").arg(d.nombre, d.creado);

    bool ok = false;
    const QString elegido = QInputDialog::getItem(this, "Cargar dibujo",
                                                  "Elegí un dibujo guardado:",
                                                  items, 0, false, &ok);
    if (!ok)
        return;
    const int idx = items.indexOf(elegido);
    if (idx < 0)
        return;

    const DibujoInfo& d = dibujos.at(idx);
    const QList<Trazo> trazos = BaseDatos::instancia().cargarDibujo(d.id);
    m_pintura->setTrazos(trazos);
    registrarEvento(QString("Dibujo cargado: '%1' (%2 trazos)").arg(d.nombre).arg(trazos.size()));
    statusBar()->showMessage(QString("Cargado: %1  (%2 trazos)").arg(d.nombre).arg(trazos.size()), 3000);
    m_pintura->setFocus();
}

void Ventana::onLimpiar() {
    m_pintura->borrarTodo();
    registrarEvento("Lienzo limpiado (boton)");
    statusBar()->showMessage("Lienzo limpiado", 2000);
    m_pintura->setFocus();
}

// ── Slots de estado del lienzo ─────────────────────────────────────────
void Ventana::onColorCambiado(QColor color) {
    QString nombre = color.name();
    if (color == QColor(Qt::black)) nombre = "negro";
    else if (color == QColor(Qt::red))   nombre = "rojo";
    else if (color == QColor(Qt::green)) nombre = "verde";
    else if (color == QColor(Qt::blue))  nombre = "azul";
    m_lblColor->setText("  Color: " + nombre + "   ");
    m_lblColor->setStyleSheet(QString("color: %1; font-weight: bold;").arg(color.name()));
}

void Ventana::onGrosorCambiado(int grosor) {
    m_lblGrosor->setText(QString("Grosor: %1 px   ").arg(grosor));
}

void Ventana::onCantidadTrazosCambiada(int cantidad) {
    m_lblTrazos->setText(QString("Trazos: %1   ").arg(cantidad));
}
