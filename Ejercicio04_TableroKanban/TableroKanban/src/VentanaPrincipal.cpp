#include "VentanaPrincipal.h"

#include <QDateTime>
#include <QStatusBar>

#include "ApiKanban.h"
#include "KanbanView.h"
#include "ModeloKanban.h"

VentanaPrincipal::VentanaPrincipal(const QString& serverUrl,
                                    const QString& basicUser,
                                    const QString& basicPass,
                                    QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Tablero Kanban Colaborativo — POO 2026");
    resize(1200, 700);

    m_modelo = new ModeloKanban(this);
    m_api    = new ApiKanban(serverUrl, basicUser, basicPass, this);
    m_kanban = new KanbanView(m_modelo, m_api, this);
    setCentralWidget(m_kanban);

    aplicarEstilo();

    m_lblEstado = new QLabel("Conectando al servidor...", this);
    statusBar()->addPermanentWidget(m_lblEstado);
    statusBar()->setSizeGripEnabled(false);

    // Cadena de datos: fetchBoard -> boardActualizado -> modelo -> tableroActualizado -> KanbanView reconstruye
    connect(m_api,    &ApiKanban::boardActualizado,
            m_modelo, &ModeloKanban::actualizarDesdeJson);

    connect(m_api, &ApiKanban::errorApi, this, [this](const QString& err) {
        m_lblEstado->setText("ERROR: " + err);
    });

    connect(m_modelo, &ModeloKanban::tableroActualizado, this, [this]() {
        m_lblEstado->setText(
            QString("Sincronizado — %1")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    });


    // Polling cada 4 segundos para actualizacion colaborativa
    connect(&m_timerPolling, &QTimer::timeout, m_api, &ApiKanban::fetchBoard);
    m_timerPolling.start(4000);

    // Primer fetch al iniciar
    QTimer::singleShot(0, m_api, &ApiKanban::fetchBoard);
}

VentanaPrincipal::~VentanaPrincipal() = default;


void VentanaPrincipal::aplicarEstilo() {
    setStyleSheet(R"QSS(
        /* Ventana principal */
        QMainWindow { background: #f0f2f5; }

        /* Toolbar Metro */
        QToolBar#toolbarPrincipal {
            background: #f3f3f3;
            border: 0;
            border-bottom: 1px solid #d0d0d0;
            padding: 4px 8px;
            spacing: 8px;
        }
        QToolBar#toolbarPrincipal QToolButton {
            background-color: #0078D7;
            color: white;
            border: 0;
            padding: 7px 20px;
            font-family: "Segoe UI";
            font-size: 10pt;
            font-weight: 600;
            letter-spacing: 1px;
        }
        QToolBar#toolbarPrincipal QToolButton:hover    { background-color: #1f8de5; }
        QToolBar#toolbarPrincipal QToolButton:pressed  { background-color: #005a9e; }

        /* Status bar */
        QStatusBar {
            background: #fafafa;
            border-top: 1px solid #e0e0e0;
            font-family: "Segoe UI";
            font-size: 9pt;
            color: #555;
        }

        /* Fondo del area kanban */
        QWidget#kanbanContenedor { background: #f0f2f5; }

        /* Columna */
        QFrame#columna {
            background: #ffffff;
            border: 1px solid #dde1e7;
            border-top: 3px solid #0078D7;
            border-radius: 4px;
        }
        QWidget#columnaHeader {
            background: transparent;
            border-bottom: 1px solid #ebebeb;
        }
        QLabel#columnaNombre {
            font-family: "Segoe UI";
            font-size: 11pt;
            font-weight: 600;
            color: #1a1a2e;
        }
        QPushButton#btnHeaderAccion {
            background: transparent;
            border: 0;
            color: #555;
            font-size: 13pt;
        }
        QPushButton#btnHeaderAccion:hover { color: #0078D7; }
        QPushButton#btnHeaderBorrar {
            background: transparent;
            border: 0;
            color: #999;
            font-size: 11pt;
        }
        QPushButton#btnHeaderBorrar:hover { color: #e53935; }

        QWidget#columnaFooter { background: transparent; }
        QPushButton#btnAgregar {
            background: transparent;
            border: 1px dashed #c0c0c0;
            color: #666;
            padding: 6px;
            font-family: "Segoe UI";
            font-size: 9pt;
        }
        QPushButton#btnAgregar:hover {
            border-color: #0078D7;
            color: #0078D7;
            background: #f0f7ff;
        }

        /* Tarjeta */
        QFrame#tarjeta {
            background: #ffffff;
            border: 1px solid #e4e8f0;
            border-left: 4px solid #0078D7;
            border-radius: 3px;
        }
        QFrame#tarjeta:hover { border-color: #b0c4de; border-left-color: #0078D7; }
        QLabel#tarjetaTitulo {
            font-family: "Segoe UI";
            font-size: 10pt;
            font-weight: 600;
            color: #1a1a2e;
        }
        QLabel#tarjetaDesc {
            font-family: "Segoe UI";
            font-size: 9pt;
            color: #777;
        }
        QPushButton#btnMover {
            background: #f0f2f5;
            border: 1px solid #ddd;
            color: #444;
            font-size: 9pt;
            border-radius: 2px;
        }
        QPushButton#btnMover:hover   { background: #d0e8ff; border-color: #0078D7; color: #0078D7; }
        QPushButton#btnMover:disabled { background: #f5f5f5; color: #ccc; border-color: #eee; }
        QPushButton#btnAccion {
            background: #f0f2f5;
            border: 1px solid #ddd;
            color: #444;
            font-family: "Segoe UI";
            font-size: 8pt;
            padding: 0 6px;
            border-radius: 2px;
        }
        QPushButton#btnAccion:hover { background: #e0eeff; color: #0078D7; }
        QPushButton#btnBorrar {
            background: #f0f2f5;
            border: 1px solid #ddd;
            color: #999;
            font-family: "Segoe UI";
            font-size: 8pt;
            padding: 0 6px;
            border-radius: 2px;
        }
        QPushButton#btnBorrar:hover { background: #ffe0e0; color: #e53935; border-color: #e53935; }

        /* Boton nueva columna */
        QPushButton#btnNuevaColumna {
            background: rgba(255,255,255,0.6);
            border: 2px dashed #b0bec5;
            color: #78909c;
            font-family: "Segoe UI";
            font-size: 10pt;
            border-radius: 4px;
        }
        QPushButton#btnNuevaColumna:hover {
            background: white;
            border-color: #0078D7;
            color: #0078D7;
        }
    )QSS");
}
