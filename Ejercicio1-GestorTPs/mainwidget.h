#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "trabajostore.h"

#include <QWidget>

class QGridLayout;
class QComboBox;
class QPlainTextEdit;
class QPushButton;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(const QString &usuario, QWidget *parent = nullptr);
    ~MainWidget() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void agregarTrabajo();
    void editarTrabajo(const QString &id);
    void eliminarTrabajo(const QString &id);
    void abrirNotas(const QString &id);
    void filtrosCambiaron();
    void onSessionExpired();

private:
    void construirUi();
    void rebuildGrid();
    void refreshHistorialUi();
    void log(const QString &mensaje);
    void scheduleSessionExpiry();
    QVector<TrabajoPractico> trabajosFiltrados() const;

    QString m_usuario;
    TrabajoStore m_store;
    QGridLayout *m_grid = nullptr;
    QComboBox *m_filtroEstado = nullptr;
    QComboBox *m_filtroPrioridad = nullptr;
    QPlainTextEdit *m_historialUi = nullptr;
    QPushButton *m_btnAgregar = nullptr;
    bool m_cierrePorExpiracion = false;
};

#endif
