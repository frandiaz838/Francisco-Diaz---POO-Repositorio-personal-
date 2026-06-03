#pragma once
#include <QWidget>
#include "Modelos.h"
#include "Pantalla.h"

class QLabel;
class QLineEdit;
class QPushButton;

// Pantalla de Login. Hereda de QWidget y de la clase abstracta Pantalla,
// implementando todos los virtuales puros.
class Login : public QWidget, public Pantalla {
    Q_OBJECT
public:
    explicit Login(const ConfigApp& config, QWidget* parent = nullptr);
    ~Login() override = default;

    // ----- Implementaciones de Pantalla (virtuales puros) -----
    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "Login"; }

signals:
    void loginExitoso();
    void bloqueoSolicitado(int segundos);

protected:
    // Redefinicion consciente de eventos (responsabilidad de Login)
    void keyPressEvent(QKeyEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    void focusInEvent(QFocusEvent* ev) override;
    void focusOutEvent(QFocusEvent* ev) override;

private slots:
    void onIngresar();

private:
    void mostrarError(const QString& msg);
    void mostrarOk(const QString& msg);

    ConfigApp    m_config;
    QLabel*      m_lblTitulo    = nullptr;
    QLabel*      m_lblSubtitulo = nullptr;
    QLineEdit*   m_inUsuario    = nullptr;
    QLineEdit*   m_inPassword   = nullptr;
    QPushButton* m_btnIngresar  = nullptr;
    QLabel*      m_lblEstado    = nullptr;

    int  m_intentosFallidos = 0;
    bool m_validado         = false;
};
