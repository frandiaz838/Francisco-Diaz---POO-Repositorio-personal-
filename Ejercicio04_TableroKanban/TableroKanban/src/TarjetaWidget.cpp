#include "TarjetaWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

TarjetaWidget::TarjetaWidget(const Tarjeta& t,
                              bool puedeIrIzq,
                              bool puedeIrDer,
                              QWidget* parent)
    : QFrame(parent), m_id(t.id)
{
    setObjectName("tarjeta");
    setFrameShape(QFrame::StyledPanel);

    auto* vlay = new QVBoxLayout(this);
    vlay->setContentsMargins(10, 8, 10, 8);
    vlay->setSpacing(4);

    // Titulo
    auto* lblTitulo = new QLabel(t.titulo, this);
    lblTitulo->setObjectName("tarjetaTitulo");
    lblTitulo->setWordWrap(true);
    vlay->addWidget(lblTitulo);

    // Descripcion (solo si tiene texto)
    if (!t.descripcion.isEmpty()) {
        auto* lblDesc = new QLabel(t.descripcion, this);
        lblDesc->setObjectName("tarjetaDesc");
        lblDesc->setWordWrap(true);
        lblDesc->setMaximumHeight(54);
        vlay->addWidget(lblDesc);
    }

    // Fila de botones
    auto* hlay = new QHBoxLayout;
    hlay->setSpacing(4);
    hlay->setContentsMargins(0, 2, 0, 0);

    auto* btnIzq = new QPushButton("◄", this);   // ◄
    btnIzq->setObjectName("btnMover");
    btnIzq->setFixedSize(28, 24);
    btnIzq->setEnabled(puedeIrIzq);
    btnIzq->setToolTip("Mover a columna anterior");

    auto* btnDer = new QPushButton("►", this);   // ►
    btnDer->setObjectName("btnMover");
    btnDer->setFixedSize(28, 24);
    btnDer->setEnabled(puedeIrDer);
    btnDer->setToolTip("Mover a columna siguiente");

    auto* btnEdit = new QPushButton("Editar", this);
    btnEdit->setObjectName("btnAccion");
    btnEdit->setFixedHeight(24);

    auto* btnDel = new QPushButton("Borrar", this);
    btnDel->setObjectName("btnBorrar");
    btnDel->setFixedHeight(24);

    hlay->addWidget(btnIzq);
    hlay->addWidget(btnDer);
    hlay->addStretch();
    hlay->addWidget(btnEdit);
    hlay->addWidget(btnDel);
    vlay->addLayout(hlay);

    connect(btnIzq,  &QPushButton::clicked, this, [this] { emit moverIzquierda(m_id); });
    connect(btnDer,  &QPushButton::clicked, this, [this] { emit moverDerecha(m_id); });
    connect(btnEdit, &QPushButton::clicked, this, [this] { emit editarPedido(m_id); });
    connect(btnDel,  &QPushButton::clicked, this, [this] { emit eliminarPedido(m_id); });
}
