#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include "MonitorController.h"

class QCloseEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void actualizarInterfaz(const QJsonObject &json);
    void mostrarError(const QString &msg);

private:
    void aplicarEstadoPanel(const QString &clave, const QString &texto);
    static QString formatearPorcentaje(double v);
    static QString formatearTextoMetrica(const QJsonObject &json,
                                         const QStringList &claves);

    MonitorController *controller;
    QLineEdit *txtUrl;
    QSpinBox *spinIntervalo;
    QSpinBox *spinUmbralCpu;
    QPushButton *btnConectar;
    QPushButton *btnDetener;
    QPushButton *btnLimpiarHistorial;
    QLabel *lblEstado;
    QLabel *lblUltimoCheck;
    QLabel *lblCpu;
    QLabel *lblCarga;
    QLabel *lblMemoria;
    QLabel *lblDisco;
    QLabel *lblUptime;
    QTextEdit *txtHistorial;
};

#endif
