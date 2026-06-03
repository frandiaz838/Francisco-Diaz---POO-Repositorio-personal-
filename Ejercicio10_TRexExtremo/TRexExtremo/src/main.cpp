#include <QApplication>
#include "Juego.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("T-Rex Extremo");

    Juego juego;
    juego.show();

    return app.exec();
}
