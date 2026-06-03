#include "ValidadorSintaxis.h"

bool ValidadorSintaxis::parentesisBalanceados(const QString& linea) const {
    // IMPORTANTE: solo se validan parentesis '()' y corchetes '[]', que en
    // la practica abren y cierran en la misma linea. Las llaves '{}' NO se
    // cuentan aqui porque delimitan bloques que abarcan varias lineas
    // (ej: "int main() {" abre en una linea y "}" cierra en otra). Contarlas
    // por linea marcaria como error casi todo el codigo valido.
    int redondos = 0, corchetes = 0;
    bool dentroString = false;
    QChar comillaActual;
    for (int i = 0; i < linea.size(); ++i) {
        QChar c = linea[i];
        if (dentroString) {
            if (c == comillaActual && (i == 0 || linea[i-1] != '\\'))
                dentroString = false;
            continue;
        }
        if (c == '"' || c == '\'') {
            dentroString = true;
            comillaActual = c;
            continue;
        }
        if (c == '(') redondos++;
        else if (c == ')') redondos--;
        else if (c == '[') corchetes++;
        else if (c == ']') corchetes--;
        if (redondos < 0 || corchetes < 0)
            return false;   // se cerro algo que nunca se abrio en esta linea
    }
    return redondos == 0 && corchetes == 0;
}

bool ValidadorSintaxis::comillasBalanceadas(const QString& linea) const {
    int dobles = 0, simples = 0;
    for (int i = 0; i < linea.size(); ++i) {
        QChar c = linea[i];
        bool escapado = (i > 0 && linea[i-1] == '\\');
        if (c == '"' && !escapado) dobles++;
        else if (c == '\'' && !escapado) simples++;
    }
    return (dobles % 2 == 0) && (simples % 2 == 0);
}

QString ValidadorSintaxis::sinComentario(const QString& linea,
                                         const QString& marca) const {
    int idx = linea.indexOf(marca);
    if (idx < 0) return linea;
    // No cortar si la marca esta dentro de un string
    bool dentroString = false;
    QChar comillaActual;
    for (int i = 0; i < idx; ++i) {
        QChar c = linea[i];
        if (dentroString) {
            if (c == comillaActual && (i == 0 || linea[i-1] != '\\'))
                dentroString = false;
        } else if (c == '"' || c == '\'') {
            dentroString = true;
            comillaActual = c;
        }
    }
    if (dentroString) return linea;
    return linea.left(idx);
}
