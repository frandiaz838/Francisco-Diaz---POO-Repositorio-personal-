#pragma once
#include <QString>
#include <QVector>

struct Tarjeta {
    int     id          = 0;
    int     columnId    = 0;
    QString titulo;
    QString descripcion;
    int     posicion    = 0;
};

struct Columna {
    int              id       = 0;
    QString          nombre;
    int              posicion = 0;
    QVector<Tarjeta> tarjetas;
};
