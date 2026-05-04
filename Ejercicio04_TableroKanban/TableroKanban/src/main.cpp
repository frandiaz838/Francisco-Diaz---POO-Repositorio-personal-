#include <QApplication>
#include "VentanaPrincipal.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Tablero Kanban");
    app.setOrganizationName("UBP-POO");

    VentanaPrincipal ventana(
        "https://167.86.77.220",  // URL base del VPS
        "poo",                     // usuario Basic Auth nginx
        "clavepoo"                 // clave  Basic Auth nginx
    );
    ventana.show();
    return app.exec();
}
