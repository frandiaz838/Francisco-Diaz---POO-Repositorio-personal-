#include "notasdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

NotasDialog::NotasDialog(const QString &tituloTp, const QString &notasIniciales, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Notas — %1").arg(tituloTp));
    setMinimumSize(520, 360);

    auto *info = new QLabel(QStringLiteral("Editá las notas y pulsá «Guardar notas» para persistir los cambios."));
    info->setWordWrap(true);

    m_edit = new QPlainTextEdit;
    m_edit->setPlainText(notasIniciales);

    auto *btnGuardar = new QPushButton(QStringLiteral("Guardar notas"));
    auto *btnCerrar = new QPushButton(QStringLiteral("Cerrar"));
    btnCerrar->setDefault(false);
    btnGuardar->setDefault(true);

    connect(btnGuardar, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::reject);

    auto *row = new QHBoxLayout;
    row->addStretch(1);
    row->addWidget(btnGuardar);
    row->addWidget(btnCerrar);

    auto *root = new QVBoxLayout(this);
    root->addWidget(info);
    root->addWidget(m_edit, 1);
    root->addLayout(row);
}

QString NotasDialog::notas() const
{
    return m_edit->toPlainText();
}
