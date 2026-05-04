#include "edittrabajodialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

EditarTrabajoDialog::EditarTrabajoDialog(const TrabajoPractico &t, QWidget *parent)
    : QDialog(parent)
    , m_id(t.id)
    , m_notas(t.notas)
{
    setWindowTitle(QStringLiteral("Editar trabajo práctico"));

    m_titulo = new QLineEdit(t.titulo);

    m_estado = new QComboBox;
    m_estado->addItems({QStringLiteral("Pendiente"),
                        QStringLiteral("En curso"),
                        QStringLiteral("Entregado")});
    m_estado->setCurrentText(t.estado.isEmpty() ? QStringLiteral("Pendiente") : t.estado);

    m_prioridad = new QComboBox;
    m_prioridad->addItems({QStringLiteral("Alta"),
                           QStringLiteral("Media"),
                           QStringLiteral("Baja")});
    m_prioridad->setCurrentText(t.prioridad.isEmpty() ? QStringLiteral("Media") : t.prioridad);

    auto *form = new QFormLayout;
    form->addRow(QStringLiteral("Título:"), m_titulo);
    form->addRow(QStringLiteral("Estado:"), m_estado);
    form->addRow(QStringLiteral("Prioridad:"), m_prioridad);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);
}

TrabajoPractico EditarTrabajoDialog::trabajo() const
{
    TrabajoPractico t;
    t.id = m_id;
    t.titulo = m_titulo->text().trimmed();
    t.estado = m_estado->currentText();
    t.prioridad = m_prioridad->currentText();
    t.notas = m_notas;
    return t;
}
