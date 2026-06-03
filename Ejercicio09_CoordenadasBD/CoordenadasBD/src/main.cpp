#include <QApplication>
#include <QMessageBox>
#include <QPointer>

#include "BaseDatos.h"
#include "Config.h"
#include "Logger.h"
#include "Login.h"
#include "Pantalla.h"
#include "Ventana.h"

// Coordinador que trabaja con punteros a la clase base Pantalla* para
// demostrar polimorfismo: el flujo central invoca metodos virtuales del
// contrato comun (nombrePantalla) sin conocer la pantalla concreta.
class Coordinador : public QObject {
    Q_OBJECT
public:
    explicit Coordinador(const ConfigApp& cfg, QObject* parent = nullptr)
        : QObject(parent), m_cfg(cfg) {
        mostrarLogin();
    }

private:
    void cambiarPantalla(QWidget* nuevoWidget, Pantalla* nuevaPantalla) {
        if (m_pantallaBase)
            Logger::instancia().registrar(
                "Coordinador: saliendo de " + m_pantallaBase->nombrePantalla());
        Logger::instancia().registrar(
            "Coordinador: ingresando a " + nuevaPantalla->nombrePantalla());

        if (m_widgetActual)
            m_widgetActual->deleteLater();

        m_widgetActual = nuevoWidget;
        m_pantallaBase = nuevaPantalla;
        m_widgetActual->show();
    }

    void mostrarLogin() {
        Login* login = new Login(m_cfg);
        connect(login, &Login::loginExitoso, this, &Coordinador::onLoginExitoso);
        cambiarPantalla(login, static_cast<Pantalla*>(login));
    }

    void mostrarVentana() {
        Ventana* ventana = new Ventana(m_cfg);
        cambiarPantalla(ventana, static_cast<Pantalla*>(ventana));
    }

private slots:
    void onLoginExitoso() { mostrarVentana(); }

private:
    ConfigApp         m_cfg;
    QPointer<QWidget> m_widgetActual = nullptr;
    Pantalla*         m_pantallaBase = nullptr;
};

#include "main.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    Logger::instancia().setRuta("app.log");
    Logger::instancia().registrar("=== Aplicacion iniciada ===");

    Config& cfg = Config::instancia();
    if (!cfg.cargar("config.ini"))
        Logger::instancia().registrar("ADVERTENCIA: config.ini no encontrado, usando defaults");

    app.setApplicationName(cfg.datos().nombreApp);
    app.setOrganizationName(cfg.datos().organizacion);

    // Abrir SQLite antes de mostrar el login (el login valida contra la base).
    if (!BaseDatos::instancia().abrir(cfg.datos().rutaDb)) {
        QMessageBox::critical(nullptr, "Base de datos",
                              "No se pudo abrir la base de datos:\n" +
                              BaseDatos::instancia().ultimoError());
        return 1;
    }

    Coordinador coordinador(cfg.datos());
    int rc = app.exec();

    Logger::instancia().registrar("=== Aplicacion finalizada ===");
    return rc;
}
