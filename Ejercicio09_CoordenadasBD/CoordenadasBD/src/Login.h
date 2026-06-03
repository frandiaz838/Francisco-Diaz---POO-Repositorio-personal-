#pragma once
#include <QWidget>
#include "Modelos.h"
#include "Pantalla.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

// Pantalla de Login construida con Qt Designer (Login.ui).
// Hereda de QWidget y de la clase abstracta Pantalla (polimorfismo).
// Valida las credenciales contra los usuarios almacenados en SQLite y
// registra en el log los accesos exitosos y los intentos fallidos.
class Login : public QWidget, public Pantalla {
    Q_OBJECT
public:
    explicit Login(const ConfigApp& config, QWidget* parent = nullptr);
    ~Login() override;

    // ----- Implementaciones de Pantalla (virtuales puros) -----
    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "Login"; }

signals:
    void loginExitoso();

protected:
    void keyPressEvent(QKeyEvent* ev) override;

private slots:
    void onIngresar();

private:
    void mostrarError(const QString& msg);
    void mostrarOk(const QString& msg);

    Ui::Login* m_ui = nullptr;
    ConfigApp  m_config;
    bool       m_validado = false;
};
