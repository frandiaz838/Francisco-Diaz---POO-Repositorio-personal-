#pragma once
#include <QString>
#include <QVector>

// Configuracion de la aplicacion, leida de config.ini.
struct ConfigApp {
    QString usuario      = "admin";              // usuario sugerido en el login
    QString rutaDb       = "coordenadas.db";     // archivo SQLite
    QString nombreApp    = "Coordenadas en Base de Datos";
    QString organizacion = "UBP-POO";
};

// Un punto del trazo, en coordenadas del lienzo.
struct Punto {
    double x = 0.0;
    double y = 0.0;
};

// Un trazo a mano alzada: color, grosor y la secuencia de puntos que
// lo componen. Persistir esta secuencia permite reconstruir el dibujo.
struct Trazo {
    QString        color  = "#000000";   // QColor::name() -> "#rrggbb"
    int            grosor = 3;
    QVector<Punto> puntos;
};

// Metadatos de un dibujo guardado en la base (para listar y elegir).
struct DibujoInfo {
    int     id = 0;
    QString nombre;
    QString creado;   // "yyyy-MM-dd hh:mm:ss" (fecha y hora de guardado)
};
