#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QString>

// Dialog reutilizable para crear y editar tarjetas.
class DialogoTarjeta : public QDialog {
    Q_OBJECT
public:
    explicit DialogoTarjeta(const QString& titulo    = {},
                             const QString& descripcion = {},
                             QWidget* parent = nullptr);

    QString titulo()      const;
    QString descripcion() const;

private:
    QLineEdit* m_edTitulo;
    QTextEdit* m_edDesc;
};
