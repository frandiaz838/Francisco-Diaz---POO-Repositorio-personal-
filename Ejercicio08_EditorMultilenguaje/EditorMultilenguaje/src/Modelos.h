#pragma once
#include <QString>

struct ConfigApp {
    QString usuario          = "admin";
    QString password         = "1234";
    int     maxIntentos      = 3;
    int     segundosBloqueo  = 30;
    QString lenguajePorDefecto = "C++";
    QString rutaExportacion  = "./exportacion.jpg";
    QString nombreApp        = "Editor Multilenguaje POO";
    QString organizacion     = "UBP-POO";
};
