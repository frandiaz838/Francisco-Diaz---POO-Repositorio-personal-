#pragma once
#include <QPixmap>
#include <QTimer>
#include <QWidget>
#include "Modelos.h"
#include "Pantalla.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class ApiClima;
class Clima;

class Login : public QWidget, public Pantalla {
    Q_OBJECT
public:
    explicit Login(const ConfigApp& config, QWidget* parent = nullptr);
    ~Login() override;

    void    inicializar()                    override;
    void    mostrarError(const QString& msg) override;
    QString nombrePantalla() const           override { return "Login"; }

signals:
    void loginExitoso(QPixmap imagenPrincipal, DatosClima clima);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void intentarLogin();
    void actualizarReloj();
    void onClimaActualizado(DatosClima datos);
    void onFondoDescargado(QPixmap px, QString ruta);
    void onImagenPrincipalDescargada(QPixmap px, QString ruta);
    void onErrorRed(QString msg);
    void tickBloqueo();

private:
    Ui::Login* m_ui;
    ConfigApp  m_config;
    ApiClima*  m_api;
    Clima*     m_clima;
    QTimer*    m_timerReloj;
    QTimer*    m_timerBloqueo;
    QPixmap    m_fondoLogin;
    QPixmap    m_imagenPrincipal;
    bool       m_loginValidado     = false;
    int        m_intentosFallidos  = 0;
    bool       m_bloqueado         = false;
    int        m_segsBloqueo       = 0;
};
