#pragma once
#include "ValidadorSintaxis.h"

class ValidadorCpp : public ValidadorSintaxis {
public:
    ResultadoValidacion validarLinea(const QString& linea) override;
    QString nombreLenguaje() const override { return "C++"; }
};
