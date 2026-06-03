#include "EditorPrincipal.h"
#include "EditorTexto.h"
#include "ExportadorJpg.h"
#include "Logger.h"
#include "ValidadorCpp.h"
#include "ValidadorJava.h"
#include "ValidadorPython.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QFocusEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>

namespace {
QLabel* mkLabel(const QString& txt, int size, const QString& color, bool bold = false) {
    QLabel* l = new QLabel(txt);
    l->setWordWrap(true);
    l->setStyleSheet(QString("color: %1; font-size: %2px; %3 background: transparent;")
                         .arg(color).arg(size)
                         .arg(bold ? "font-weight: bold;" : ""));
    return l;
}
QFrame* mkCard() {
    QFrame* f = new QFrame;
    f->setStyleSheet(
        "QFrame { background-color: rgba(255,255,255,12); "
        "border-radius: 14px; border: 1px solid rgba(74,144,226,40); }");
    return f;
}
QPushButton* mkBoton(const QString& txt, const QString& bg) {
    QPushButton* b = new QPushButton(txt);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString(
        "QPushButton { background: %1; color: white; border: 1px solid rgba(255,255,255,40); "
        "border-radius: 8px; padding: 8px 16px; font-size: 13px; font-weight: bold; } "
        "QPushButton:hover { background: rgba(74,144,226,200); }").arg(bg));
    return b;
}
} // namespace

EditorPrincipal::EditorPrincipal(const ConfigApp& config, QWidget* parent)
    : QMainWindow(parent), m_config(config) {
    setWindowTitle("Editor Multilenguaje POO");
    setStyleSheet("QMainWindow { background: #111122; } "
                  "QStatusBar { background: #1a1a2e; color: #aaaaaa; }");

    inicializarUI();
    conectarEventos();
    cargarDatos();
    showFullScreen();
}

void EditorPrincipal::inicializarUI() {
    QWidget* central = new QWidget;
    setCentralWidget(central);

    QVBoxLayout* root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Barra superior ────────────────────────────────────────────────
    QFrame* bar = new QFrame;
    bar->setStyleSheet("QFrame { background: rgba(0,0,0,140); "
                       "border-bottom: 1px solid rgba(74,144,226,80); }");
    QHBoxLayout* h = new QHBoxLayout(bar);
    h->setContentsMargins(24, 12, 24, 12);
    h->setSpacing(16);

    QLabel* titulo = new QLabel("Editor Multilenguaje");
    titulo->setStyleSheet("color: white; font-size: 16px; font-weight: bold; "
                          "background: transparent;");
    h->addWidget(titulo);

    QLabel* lblLeng = new QLabel("Lenguaje:");
    lblLeng->setStyleSheet("color: #cccccc; font-size: 13px; background: transparent;");
    h->addSpacing(20);
    h->addWidget(lblLeng);

    m_cmbLenguaje = new QComboBox;
    m_cmbLenguaje->addItems({"C++", "Python", "Java"});
    m_cmbLenguaje->setStyleSheet(
        "QComboBox { background: rgba(255,255,255,15); color: white; "
        "border: 1px solid rgba(74,144,226,80); border-radius: 6px; "
        "padding: 6px 14px; font-size: 13px; min-width: 110px; } "
        "QComboBox QAbstractItemView { background: #1e1e2e; color: white; "
        "selection-background-color: #4A90E2; }");
    h->addWidget(m_cmbLenguaje);

    h->addStretch();

    m_btnExportar = mkBoton("Exportar a JPG", "rgba(74,144,226,160)");
    h->addWidget(m_btnExportar);

    m_btnSalir = mkBoton("Salir", "rgba(220,80,80,160)");
    h->addWidget(m_btnSalir);

    root->addWidget(bar);

    // ── Cuerpo: splitter editor / CV ──────────────────────────────────
    QSplitter* split = new QSplitter(Qt::Horizontal);
    split->setStyleSheet("QSplitter::handle { background: rgba(74,144,226,60); width: 2px; }");

    // Izquierda: editor con diagnostico
    QWidget* izq = new QWidget;
    izq->setStyleSheet("background: #111122;");
    QVBoxLayout* lyEd = new QVBoxLayout(izq);
    lyEd->setContentsMargins(16, 16, 16, 16);
    lyEd->setSpacing(8);

    m_editor = new EditorTexto;
    lyEd->addWidget(m_editor, 1);

    m_lblDiagnostico = new QLabel("Sin errores");
    m_lblDiagnostico->setStyleSheet(
        "color: #80ffaa; font-size: 12px; background: rgba(0,0,0,80); "
        "border-radius: 6px; padding: 8px 12px;");
    lyEd->addWidget(m_lblDiagnostico);

    split->addWidget(izq);
    split->addWidget(construirPanelCv());
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);

    root->addWidget(split, 1);

    statusBar()->showMessage("Listo");
}

void EditorPrincipal::conectarEventos() {
    // signals/slots con sintaxis moderna
    connect(m_cmbLenguaje, &QComboBox::currentTextChanged,
            this, &EditorPrincipal::onLenguajeCambiado);
    connect(m_btnExportar, &QPushButton::clicked,
            this, &EditorPrincipal::onExportarJpg);
    connect(m_btnSalir, &QPushButton::clicked,
            this, &EditorPrincipal::onSalir);
    connect(m_editor, &EditorTexto::validacionLinea,
            this, &EditorPrincipal::onValidacionLinea);
}

void EditorPrincipal::cargarDatos() {
    aplicarValidador(m_config.lenguajePorDefecto);
    m_cmbLenguaje->setCurrentText(m_config.lenguajePorDefecto);

    // Texto de ejemplo segun lenguaje
    m_editor->setPlainText(
        "// Editor Multilenguaje - escriba aqui su codigo\n"
        "// Cambie el lenguaje en el selector superior\n"
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "    std::cout << \"Hola UBP\" << std::endl;\n"
        "    return 0;\n"
        "}\n");

    m_editor->revalidarTodo();
    registrarEvento("Editor iniciado con lenguaje " + m_config.lenguajePorDefecto);
}

bool EditorPrincipal::validarEstado() {
    return m_editor != nullptr && m_validador != nullptr;
}

void EditorPrincipal::registrarEvento(const QString& descripcion) {
    Logger::instancia().registrar("[" + nombrePantalla() + "] " + descripcion);
}

void EditorPrincipal::aplicarValidador(const QString& lenguaje) {
    if (lenguaje == "Python")
        m_validador = std::make_unique<ValidadorPython>();
    else if (lenguaje == "Java")
        m_validador = std::make_unique<ValidadorJava>();
    else
        m_validador = std::make_unique<ValidadorCpp>();

    // Polimorfismo: el editor recibe el puntero a la clase base
    m_editor->setValidador(m_validador.get());
}

void EditorPrincipal::onLenguajeCambiado(const QString& lenguaje) {
    aplicarValidador(lenguaje);
    statusBar()->showMessage("Lenguaje cambiado a " + lenguaje, 3000);
    registrarEvento("Lenguaje cambiado a " + lenguaje);
}

void EditorPrincipal::onValidacionLinea(int numeroLinea, bool valida,
                                        const QString& mensaje) {
    if (valida) {
        m_lblDiagnostico->setStyleSheet(
            "color: #80ffaa; font-size: 12px; background: rgba(0,0,0,80); "
            "border-radius: 6px; padding: 8px 12px;");
        m_lblDiagnostico->setText(
            QString("Linea %1 OK").arg(numeroLinea + 1));
    } else {
        m_lblDiagnostico->setStyleSheet(
            "color: #ff8080; font-size: 12px; background: rgba(80,0,0,120); "
            "border-radius: 6px; padding: 8px 12px;");
        m_lblDiagnostico->setText(
            QString("Linea %1: %2").arg(numeroLinea + 1).arg(mensaje));
        registrarEvento(QString("Validacion fallo en linea %1: %2")
                            .arg(numeroLinea + 1).arg(mensaje));
    }
}

void EditorPrincipal::onExportarJpg() {
    QString ruta = m_config.rutaExportacion;
    bool ok = ExportadorJpg::exportar(
        m_editor->toPlainText(),
        m_validador ? m_validador->nombreLenguaje() : "Texto",
        ruta);
    if (ok) {
        statusBar()->showMessage("Exportado a " + ruta, 5000);
        registrarEvento("Exportacion JPG completada: " + ruta);
        QMessageBox::information(this, "Exportar",
                                  "Código exportado correctamente a:\n" + ruta);
    } else {
        statusBar()->showMessage("Error exportando JPG", 5000);
        registrarEvento("Exportacion JPG fallo en: " + ruta);
        QMessageBox::warning(this, "Exportar",
                              "No se pudo exportar a:\n" + ruta);
    }
}

void EditorPrincipal::onSalir() {
    close();
}

// ── Eventos redefinidos (responsabilidad de EditorPrincipal) ──────────
void EditorPrincipal::keyPressEvent(QKeyEvent* ev) {
    // Ctrl+E: exportar
    if (ev->modifiers().testFlag(Qt::ControlModifier) && ev->key() == Qt::Key_E) {
        onExportarJpg();
        return;
    }
    // Ctrl+L: cambiar lenguaje (cicla en el combo)
    if (ev->modifiers().testFlag(Qt::ControlModifier) && ev->key() == Qt::Key_L) {
        int next = (m_cmbLenguaje->currentIndex() + 1) % m_cmbLenguaje->count();
        m_cmbLenguaje->setCurrentIndex(next);
        return;
    }
    // F11: alternar fullscreen
    if (ev->key() == Qt::Key_F11) {
        if (isFullScreen()) showNormal();
        else showFullScreen();
        registrarEvento("F11: cambio de modo fullscreen");
        return;
    }
    QMainWindow::keyPressEvent(ev);
}

void EditorPrincipal::mousePressEvent(QMouseEvent* ev) {
    // En EditorPrincipal el click sobre el fondo enfoca el editor
    if (ev->button() == Qt::LeftButton && m_editor)
        m_editor->setFocus();
    QMainWindow::mousePressEvent(ev);
}

void EditorPrincipal::resizeEvent(QResizeEvent* ev) {
    // En EditorPrincipal aprovechamos resize para registrarlo
    registrarEvento(QString("Resize a %1x%2")
                        .arg(ev->size().width()).arg(ev->size().height()));
    QMainWindow::resizeEvent(ev);
}

void EditorPrincipal::closeEvent(QCloseEvent* ev) {
    // Confirmacion de salida y oferta de guardado
    auto r = QMessageBox::question(
        this, "Salir",
        "¿Desea exportar el código a JPG antes de salir?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (r == QMessageBox::Cancel) {
        ev->ignore();
        registrarEvento("Cierre cancelado por el usuario");
        return;
    }
    if (r == QMessageBox::Yes)
        onExportarJpg();

    registrarEvento("Cierre confirmado");
    QMainWindow::closeEvent(ev);
}

void EditorPrincipal::focusInEvent(QFocusEvent* ev) {
    // EditorPrincipal usa focusIn para reactivar la validacion completa
    if (m_editor)
        m_editor->revalidarTodo();
    registrarEvento("EditorPrincipal recibio foco - revalidacion ejecutada");
    QMainWindow::focusInEvent(ev);
}

void EditorPrincipal::focusOutEvent(QFocusEvent* ev) {
    registrarEvento("EditorPrincipal perdio foco");
    QMainWindow::focusOutEvent(ev);
}

// ── Panel CV LinkedIn ─────────────────────────────────────────────────
QWidget* EditorPrincipal::construirPanelCv() {
    QScrollArea* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea { background: #15152a; border: none; } "
        "QScrollBar:vertical { background: rgba(255,255,255,15); "
        "width: 8px; border-radius: 4px; }");

    QWidget* contenedor = new QWidget;
    contenedor->setStyleSheet("background: #15152a;");
    scroll->setWidget(contenedor);

    QVBoxLayout* lay = new QVBoxLayout(contenedor);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);

    // Header: foto (iniciales) + nombre
    QFrame* header = mkCard();
    QHBoxLayout* hh = new QHBoxLayout(header);
    hh->setContentsMargins(20, 20, 20, 20);
    hh->setSpacing(16);

    QLabel* foto = new QLabel("OB");
    foto->setFixedSize(80, 80);
    foto->setAlignment(Qt::AlignCenter);
    foto->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #4A90E2,stop:1 #7B68EE); border-radius: 40px; "
        "color: white; font-size: 26px; font-weight: bold;");

    QWidget* infoW = new QWidget;
    infoW->setStyleSheet("background: transparent;");
    QVBoxLayout* iv = new QVBoxLayout(infoW);
    iv->setContentsMargins(0, 0, 0, 0);
    iv->setSpacing(3);
    iv->addWidget(mkLabel("Octavio Brachetti", 18, "white", true));
    iv->addWidget(mkLabel("Estudiante de Ingeniería en Software", 12, "#4A90E2"));
    iv->addWidget(mkLabel("UBP - Córdoba, Argentina", 11, "#aaaaaa"));

    hh->addWidget(foto);
    hh->addWidget(infoW, 1);
    lay->addWidget(header);

    auto seccion = [&](const QString& titulo) -> QVBoxLayout* {
        QFrame* card = mkCard();
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(20, 14, 20, 14);
        cl->setSpacing(8);

        QLabel* t = new QLabel(titulo);
        t->setStyleSheet("color: #4A90E2; font-size: 13px; font-weight: bold; "
                         "background: transparent; border: none;");
        cl->addWidget(t);

        QFrame* sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("color: rgba(74,144,226,60);");
        cl->addWidget(sep);
        lay->addWidget(card);
        return cl;
    };

    QVBoxLayout* s1 = seccion("Sobre mí");
    s1->addWidget(mkLabel(
        "Estudiante de 3er año de Ingeniería en Software en UBP. "
        "Apasionado por C++/Qt, desarrollo backend con Python/FastAPI "
        "y arquitectura orientada a objetos.",
        12, "#dddddd"));

    QVBoxLayout* s2 = seccion("Habilidades");
    QWidget* skillW = new QWidget;
    skillW->setStyleSheet("background: transparent;");
    QGridLayout* sg = new QGridLayout(skillW);
    sg->setContentsMargins(0, 0, 0, 0);
    sg->setSpacing(6);
    QStringList skills = {"C++17", "Qt6", "Python", "Java", "FastAPI",
                          "MySQL", "Docker", "Git", "CMake", "POO"};
    for (int i = 0; i < skills.size(); ++i) {
        QLabel* sk = new QLabel(skills[i]);
        sk->setAlignment(Qt::AlignCenter);
        sk->setStyleSheet(
            "background: rgba(74,144,226,25); color: #4A90E2; "
            "border: 1px solid rgba(74,144,226,55); border-radius: 5px; "
            "padding: 5px 10px; font-size: 11px;");
        sg->addWidget(sk, i / 3, i % 3);
    }
    s2->addWidget(skillW);

    QVBoxLayout* s3 = seccion("Proyectos");
    for (const auto& p : QList<QPair<QString, QString>>{
             {"Tablero Kanban",    "Qt6 + FastAPI + MySQL en VPS Docker"},
             {"Login con Clima",   "Qt6 + OpenWeatherMap, cache offline"},
             {"Editor Multilenguaje", "POO con polimorfismo, validadores"},
         }) {
        QFrame* pf = new QFrame;
        pf->setStyleSheet(
            "QFrame { background: rgba(74,144,226,15); border-radius: 8px; "
            "border: 1px solid rgba(74,144,226,40); }");
        QVBoxLayout* pl = new QVBoxLayout(pf);
        pl->setContentsMargins(12, 8, 12, 8);
        pl->setSpacing(3);
        pl->addWidget(mkLabel(p.first, 12, "white", true));
        pl->addWidget(mkLabel(p.second, 11, "#bbbbbb"));
        s3->addWidget(pf);
    }

    QVBoxLayout* s4 = seccion("Contacto");
    s4->addWidget(mkLabel("octaviobrachetti210@gmail.com", 12, "#dddddd"));
    s4->addWidget(mkLabel("github.com/octaviobrachetti",   12, "#dddddd"));
    s4->addWidget(mkLabel("UBP - Córdoba, Argentina",       12, "#aaaaaa"));

    lay->addStretch();
    return scroll;
}
