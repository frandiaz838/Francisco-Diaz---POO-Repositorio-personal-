#ifndef NOTASDIALOG_H
#define NOTASDIALOG_H

#include <QDialog>

class QPlainTextEdit;

class NotasDialog : public QDialog
{
public:
    NotasDialog(const QString &tituloTp, const QString &notasIniciales, QWidget *parent = nullptr);

    QString notas() const;

private:
    QPlainTextEdit *m_edit = nullptr;
};

#endif
