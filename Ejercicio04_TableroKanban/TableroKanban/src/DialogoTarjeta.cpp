#include "DialogoTarjeta.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DialogoTarjeta::DialogoTarjeta(const QString& titulo,
                                const QString& descripcion,
                                QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(titulo.isEmpty() ? "Nueva tarjeta" : "Editar tarjeta");
    setMinimumWidth(360);

    auto* vlay = new QVBoxLayout(this);
    auto* form = new QFormLayout;

    m_edTitulo = new QLineEdit(titulo, this);
    m_edTitulo->setPlaceholderText("Titulo de la tarjeta");
    form->addRow("Titulo:", m_edTitulo);

    m_edDesc = new QTextEdit(this);
    m_edDesc->setPlainText(descripcion);
    m_edDesc->setPlaceholderText("Descripcion (opcional)");
    m_edDesc->setFixedHeight(90);
    form->addRow("Descripcion:", m_edDesc);

    vlay->addLayout(form);

    auto* btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btns->button(QDialogButtonBox::Ok)->setText("Guardar");
    btns->button(QDialogButtonBox::Cancel)->setText("Cancelar");
    vlay->addWidget(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_edTitulo->setFocus();
}

QString DialogoTarjeta::titulo() const {
    return m_edTitulo->text().trimmed();
}

QString DialogoTarjeta::descripcion() const {
    return m_edDesc->toPlainText().trimmed();
}
