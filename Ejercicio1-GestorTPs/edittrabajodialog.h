#ifndef EDITTRABAJODIALOG_H
#define EDITTRABAJODIALOG_H

#include "trabajo.h"

#include <QDialog>

class QLineEdit;
class QComboBox;

class EditarTrabajoDialog : public QDialog
{
public:
    EditarTrabajoDialog(const TrabajoPractico &t, QWidget *parent = nullptr);

    TrabajoPractico trabajo() const;

private:
    QLineEdit *m_titulo = nullptr;
    QComboBox *m_estado = nullptr;
    QComboBox *m_prioridad = nullptr;
    QString m_id;
    QString m_notas;
};

#endif
