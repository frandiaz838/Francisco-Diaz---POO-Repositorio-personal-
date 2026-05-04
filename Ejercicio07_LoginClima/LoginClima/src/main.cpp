#include <QApplication>
#include "Config.h"
#include "Logger.h"
#include "Login.h"
#include "Ventana.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LoginClima");
    app.setOrganizationName("UBP-POO");

    Logger::instancia().setRuta("app.log");
    Logger::instancia().registrar("=== Aplicacion iniciada ===");

    Config& cfg = Config::instancia();
    if (!cfg.cargar("config.ini"))
        Logger::instancia().registrar("ADVERTENCIA: config.ini no encontrado, se usaran defaults");

    Login* login = new Login(cfg.datos());

    QObject::connect(login, &Login::loginExitoso,
                     [login, &cfg](QPixmap imagen, DatosClima clima) {
        Ventana* ventana = new Ventana(imagen, clima, cfg.datos());
        ventana->show();
        login->deleteLater();
    });

    login->show();
    return app.exec();
}
