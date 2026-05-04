#pragma once
#include <QMainWindow>
#include <QPixmap>
#include "Modelos.h"
#include "Pantalla.h"

class QLabel;
class QPushButton;
class QStackedWidget;
class QTimer;
class QVBoxLayout;
class ApiClima;
class Clima;

class Ventana : public QMainWindow, public Pantalla {
    Q_OBJECT
public:
    Ventana(const QPixmap& imagen,
            const DatosClima& clima,
            const ConfigApp& config,
            QWidget* parent = nullptr);

    void    inicializar()                    override;
    void    mostrarError(const QString& msg) override;
    QString nombrePantalla() const           override { return "Ventana"; }

private slots:
    void onToggleVista();
    void onRefrescarClima();
    void onClimaActualizado(DatosClima d);
    void onErrorRed(QString msg);
    void actualizarReloj();
    void salir();

private:
    void     construirHeader(QVBoxLayout* parentLay);
    QWidget* construirDashboardClima();
    QWidget* construirCV();
    void     renderClima();

    QPixmap         m_imagen;
    ConfigApp       m_config;
    DatosClima      m_datosClima;

    ApiClima*       m_api            = nullptr;
    Clima*          m_clima          = nullptr;

    QStackedWidget* m_stack          = nullptr;
    QPushButton*    m_btnVista       = nullptr;
    QLabel*         m_lblHora        = nullptr;
    QTimer*         m_timerReloj     = nullptr;

    // Widgets del dashboard de clima (refrescables)
    QLabel*         m_lblTemp        = nullptr;
    QLabel*         m_lblCiudad      = nullptr;
    QLabel*         m_lblDesc        = nullptr;
    QLabel*         m_lblActualizado = nullptr;
    QLabel*         m_lblEstado      = nullptr;
    QPushButton*    m_btnRefrescar   = nullptr;
};
