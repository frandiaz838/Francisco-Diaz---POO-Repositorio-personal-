#pragma once
#include <QString>

// Utilidad sin estado para exportar el contenido del editor a un unico
// archivo JPG con texto legible y respetando saltos de linea.
class ExportadorJpg {
public:
    static bool exportar(const QString& codigo,
                         const QString& lenguaje,
                         const QString& rutaSalida);
};
