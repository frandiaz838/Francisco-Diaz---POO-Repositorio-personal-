#include <QApplication>
#include <QPointer>

#include "Config.h"
#include "EditorPrincipal.h"
#include "Logger.h"
#include "Login.h"
#include "ModoBloqueado.h"
#include "Pantalla.h"

// Coordinador minimo que trabaja con punteros a la clase base Pantalla*
// para demostrar polimorfismo: el flujo central no conoce la pantalla
// concreta, solo invoca metodos virtuales del contrato comun.
class Coordinador : public QObject {
    Q_OBJECT
public:
    explicit Coordinador(const ConfigApp& cfg, QObject* parent = nullptr)
        : QObject(parent), m_cfg(cfg) {
        mostrarLogin();
    }

private:
    void cambiarPantalla(QWidget* nuevoWidget, Pantalla* nuevaPantalla) {
        // Polimorfismo: registramos el cambio invocando nombrePantalla() virtual
        if (m_pantallaBase) {
            Logger::instancia().registrar(
                "Coordinador: saliendo de " + m_pantallaBase->nombrePantalla());
        }
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
        connect(login, &Login::bloqueoSolicitado, this, &Coordinador::onBloqueo);
        // Pasamos como Pantalla* (clase base) → polimorfismo
        cambiarPantalla(login, static_cast<Pantalla*>(login));
    }

    void mostrarEditor() {
        EditorPrincipal* editor = new EditorPrincipal(m_cfg);
        cambiarPantalla(editor, static_cast<Pantalla*>(editor));
    }

    void mostrarBloqueado(int segundos) {
        ModoBloqueado* bloq = new ModoBloqueado(segundos);
        connect(bloq, &ModoBloqueado::desbloqueado,
                this, &Coordinador::mostrarLogin);
        cambiarPantalla(bloq, static_cast<Pantalla*>(bloq));
    }

private slots:
    void onLoginExitoso()         { mostrarEditor(); }
    void onBloqueo(int segundos)  { mostrarBloqueado(segundos); }

private:
    ConfigApp        m_cfg;
    QPointer<QWidget> m_widgetActual = nullptr;
    Pantalla*        m_pantallaBase  = nullptr;
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

    Coordinador coordinador(cfg.datos());
    int rc = app.exec();

    Logger::instancia().registrar("=== Aplicacion finalizada ===");
    return rc;
}
