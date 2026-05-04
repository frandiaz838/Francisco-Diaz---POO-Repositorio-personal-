#include "KanbanView.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "ApiKanban.h"
#include "ColumnaWidget.h"
#include "ModeloKanban.h"

KanbanView::KanbanView(ModeloKanban* modelo, ApiKanban* api, QWidget* parent)
    : QScrollArea(parent), m_modelo(modelo), m_api(api)
{
    setObjectName("kanbanView");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
    setFrameShape(QFrame::NoFrame);

    m_contenedor = new QWidget(this);
    m_contenedor->setObjectName("kanbanContenedor");
    m_hlay = new QHBoxLayout(m_contenedor);
    m_hlay->setContentsMargins(16, 16, 16, 16);
    m_hlay->setSpacing(12);
    m_hlay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setWidget(m_contenedor);

    connect(m_modelo, &ModeloKanban::tableroActualizado,
            this,     &KanbanView::reconstruir);

    // Si la API confirma algo, hacer fetchBoard para refrescar
    connect(m_api, &ApiKanban::operacionOk,
            m_api, &ApiKanban::fetchBoard);
}

void KanbanView::reconstruir() {
    // Eliminar todos los widgets actuales del layout
    QLayoutItem* item;
    while ((item = m_hlay->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    const QVector<Columna>& cols = m_modelo->columnas();
    const int total = cols.size();

    for (int i = 0; i < total; ++i) {
        int prevId = (i > 0)          ? cols[i - 1].id : -1;
        int nextId = (i < total - 1)  ? cols[i + 1].id : -1;

        auto* cw = new ColumnaWidget(cols[i], prevId, nextId, m_api, m_contenedor);

        // Cuando cualquier columna reporte operacion OK, fetchBoard ya fue
        // conectado arriba (api::operacionOk -> api::fetchBoard).
        // Solo necesitamos que el modelo se actualice para redibujar.
        // La cadena: operacionOk -> fetchBoard -> boardActualizado -> actualizarDesdeJson -> tableroActualizado -> reconstruir

        m_hlay->addWidget(cw);
    }

    // Boton para agregar nueva columna al final
    auto* btnNueva = new QPushButton("＋\nColumna", m_contenedor);
    btnNueva->setObjectName("btnNuevaColumna");
    btnNueva->setFixedSize(100, 80);
    connect(btnNueva, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString nombre = QInputDialog::getText(
            this, "Nueva columna", "Nombre de la columna:",
            QLineEdit::Normal, {}, &ok);
        if (ok && !nombre.trimmed().isEmpty())
            m_api->crearColumna(nombre.trimmed());
    });
    m_hlay->addWidget(btnNueva, 0, Qt::AlignTop);

    // Ajustar el ancho del contenedor al contenido
    int ancho = 16 + total * (268 + 12) + 100 + 12 + 16;
    m_contenedor->setMinimumWidth(ancho);
}
