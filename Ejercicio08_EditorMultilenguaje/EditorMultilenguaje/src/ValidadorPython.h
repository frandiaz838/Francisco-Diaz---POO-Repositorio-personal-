#pragma once
#include "ValidadorSintaxis.h"

class ValidadorPython : public ValidadorSintaxis {
public:
    ResultadoValidacion validarLinea(const QString& linea) override;
    QString nombreLenguaje() const override { return "Python"; }
};
