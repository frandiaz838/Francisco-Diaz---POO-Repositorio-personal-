#pragma once
#include "ValidadorSintaxis.h"

class ValidadorJava : public ValidadorSintaxis {
public:
    ResultadoValidacion validarLinea(const QString& linea) override;
    QString nombreLenguaje() const override { return "Java"; }
};
