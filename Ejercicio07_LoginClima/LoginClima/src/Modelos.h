#pragma once
#include <QString>

struct DatosClima {
    double  temperatura  = 0.0;
    QString descripcion;
    QString ciudad;
    bool    valido       = false;
};

struct ConfigApp {
    QString apiKey;
    QString ciudad;
    QString unidades;
    QString urlFondoLogin;
    QString urlImagenPrincipal;
    bool    proxyHabilitado = false;
    QString proxyHost;
    int     proxyPuerto    = 8080;
};
