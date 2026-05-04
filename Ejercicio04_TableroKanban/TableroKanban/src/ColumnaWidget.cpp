#include "ColumnaWidget.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "ApiKanban.h"
#include "DialogoTarjeta.h"
#include "TarjetaWidget.h"

ColumnaWidget::ColumnaWidget(const Columna& col,
                              int prevColId,
                              int nextColId,
                              ApiKanban* api,
                              QWidget* parent)
    : QFrame(parent), m_col(col), m_prevColId(prevColId), m_nextColId(nextColId), m_api(api)
{
    setObjectName("columna");
    setFrameShape(QFrame::StyledPanel);
    setFixedWidth(268);

    auto* vlay = new QVBoxLayout(this);
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    // ── Encabezado ─────────────────────────────────────────────────────────
    auto* header = new QWidget(this);
    header->setObjectName("columnaHeader");
    auto* hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(10, 8, 8, 8);

    auto* lblNombre = new QLabel(col.nombre, header);
    lblNombre->setObjectName("columnaNombre");
    hlay->addWidget(lblNombre, 1);

    auto* btnEdit = new QPushButton("✎", header);
    btnEdit->setObjectName("btnHeaderAccion");
    btnEdit->setFixedSize(26, 26);
    btnEdit->setToolTip("Renombrar columna");
    hlay->addWidget(btnEdit);

    auto* btnDel = new QPushButton("✕", header);
    btnDel->setObjectName("btnHeaderBorrar");
    btnDel->setFixedSize(26, 26);
    btnDel->setToolTip("Eliminar columna");
    hlay->addWidget(btnDel);

    vlay->addWidget(header);

    // ── Area scrollable de tarjetas ─────────────────────────────────────────
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* contenedor = new QWidget(scroll);
    contenedor->setObjectName("contenedorTarjetas");
    auto* tarjetasLayout = new QVBoxLayout(contenedor);
    tarjetasLayout->setContentsMargins(8, 8, 8, 8);
    tarjetasLayout->setSpacing(8);
    tarjetasLayout->setAlignment(Qt::AlignTop);

    bool puedeIzq = (prevColId != -1);
    bool puedeDer = (nextColId != -1);

    for (const Tarjeta& t : col.tarjetas) {
        auto* tw = new TarjetaWidget(t, puedeIzq, puedeDer, contenedor);
        tarjetasLayout->addWidget(tw);

        connect(tw, &TarjetaWidget::editarPedido,
                this, &ColumnaWidget::pedirEditarTarjeta);
        connect(tw, &TarjetaWidget::eliminarPedido,
                this, &ColumnaWidget::pedirEliminarTarjeta);
        connect(tw, &TarjetaWidget::moverIzquierda,
                this, &ColumnaWidget::pedirMoverTarjetaIzq);
        connect(tw, &TarjetaWidget::moverDerecha,
                this, &ColumnaWidget::pedirMoverTarjetaDer);
    }

    scroll->setWidget(contenedor);
    vlay->addWidget(scroll, 1);

    // ── Boton agregar tarjeta ───────────────────────────────────────────────
    auto* footer = new QWidget(this);
    footer->setObjectName("columnaFooter");
    auto* flay = new QHBoxLayout(footer);
    flay->setContentsMargins(8, 6, 8, 8);

    auto* btnAgregar = new QPushButton("+ Agregar tarjeta", footer);
    btnAgregar->setObjectName("btnAgregar");
    flay->addWidget(btnAgregar);

    vlay->addWidget(footer);

    // ── Conexiones ──────────────────────────────────────────────────────────
    connect(btnEdit,    &QPushButton::clicked, this, &ColumnaWidget::pedirEditarColumna);
    connect(btnDel,     &QPushButton::clicked, this, &ColumnaWidget::pedirEliminarColumna);
    connect(btnAgregar, &QPushButton::clicked, this, &ColumnaWidget::pedirNuevaTarjeta);

    // Cuando la API confirma algo que ocurrio en esta columna, lo propagamos
    // a KanbanView (que conectara operacionCompletada a fetchBoard).
    // NO conectar operacionOk directamente aqui para evitar fetch doble.
}

void ColumnaWidget::pedirNuevaTarjeta() {
    ApiKanban* api = m_api;
    int colId = m_col.id;
    DialogoTarjeta dlg({}, {}, window());
    if (dlg.exec() != QDialog::Accepted) return;
    if (dlg.titulo().isEmpty()) return;
    api->crearTarjeta(colId, dlg.titulo(), dlg.descripcion());
}

void ColumnaWidget::pedirEditarColumna() {
    ApiKanban* api = m_api;
    int colId = m_col.id;
    QString nombreActual = m_col.nombre;
    bool ok;
    QString nuevo = QInputDialog::getText(
        window(), "Renombrar columna", "Nuevo nombre:",
        QLineEdit::Normal, nombreActual, &ok);
    if (!ok || nuevo.trimmed().isEmpty()) return;
    api->editarColumna(colId, nuevo.trimmed());
}

void ColumnaWidget::pedirEliminarColumna() {
    ApiKanban* api = m_api;
    int colId = m_col.id;
    QString nombreCol = m_col.nombre;
    int resp = QMessageBox::question(
        window(), "Eliminar columna",
        QString("Eliminar la columna \"%1\" y todas sus tarjetas?").arg(nombreCol),
        QMessageBox::Yes | QMessageBox::No);
    if (resp != QMessageBox::Yes) return;
    api->eliminarColumna(colId);
}

void ColumnaWidget::pedirEditarTarjeta(int cardId) {
    ApiKanban* api = m_api;
    for (const Tarjeta& t : m_col.tarjetas) {
        if (t.id == cardId) {
            QString titulo = t.titulo;
            QString descripcion = t.descripcion;
            DialogoTarjeta dlg(titulo, descripcion, window());
            if (dlg.exec() != QDialog::Accepted) return;
            if (dlg.titulo().isEmpty()) return;
            api->editarTarjeta(cardId, dlg.titulo(), dlg.descripcion());
            return;
        }
    }
}

void ColumnaWidget::pedirEliminarTarjeta(int cardId) {
    ApiKanban* api = m_api;
    int resp = QMessageBox::question(
        window(), "Eliminar tarjeta",
        "Eliminar esta tarjeta?",
        QMessageBox::Yes | QMessageBox::No);
    if (resp != QMessageBox::Yes) return;
    api->eliminarTarjeta(cardId);
}

void ColumnaWidget::pedirMoverTarjetaIzq(int cardId) {
    if (m_prevColId == -1) return;
    m_api->moverTarjeta(cardId, m_prevColId);
}

void ColumnaWidget::pedirMoverTarjetaDer(int cardId) {
    if (m_nextColId == -1) return;
    m_api->moverTarjeta(cardId, m_nextColId);
}
