#pragma once
#include <QWidget>
#include "Pantalla.h"

class QLabel;
class QTimer;

// Pantalla ModoBloqueado. Se muestra tras superar la cantidad maxima
// de intentos fallidos en Login. Cuenta regresiva y al terminar emite
// la senal desbloqueado() para volver a Login.
class ModoBloqueado : public QWidget, public Pantalla {
    Q_OBJECT
public:
    explicit ModoBloqueado(int segundos, QWidget* parent = nullptr);

    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "ModoBloqueado"; }

signals:
    void desbloqueado();

protected:
    void keyPressEvent(QKeyEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    void focusInEvent(QFocusEvent* ev) override;
    void focusOutEvent(QFocusEvent* ev) override;

private slots:
    void onTick();

private:
    int     m_segundosRestantes = 0;
    QLabel* m_lblTitulo         = nullptr;
    QLabel* m_lblCountdown      = nullptr;
    QLabel* m_lblMensaje        = nullptr;
    QTimer* m_timer             = nullptr;
};
