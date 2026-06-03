#include "ValidadorPython.h"
#include <QRegularExpression>

ResultadoValidacion ValidadorPython::validarLinea(const QString& lineaOriginal) {
    ResultadoValidacion r;
    QString linea = sinComentario(lineaOriginal, "#").trimmed();
    if (linea.isEmpty()) return r;

    if (!comillasBalanceadas(linea)) {
        r.valido  = false;
        r.mensaje = "Comillas sin cerrar en la línea.";
        return r;
    }
    if (!parentesisBalanceados(linea)) {
        r.valido  = false;
        r.mensaje = "Paréntesis o corchetes desbalanceados.";
        return r;
    }

    // Estructuras que requieren ':' al final
    static const QRegularExpression rxBloque(
        R"(^(if|elif|else|for|while|def|class|try|except|finally|with)\b)");
    if (rxBloque.match(linea).hasMatch()) {
        // Error tipico: usar llave estilo C en vez de ':' para abrir el bloque
        if (linea.endsWith('{')) {
            r.valido  = false;
            r.mensaje = "Python no usa llaves '{'. Termine el bloque con ':' e indentación.";
            return r;
        }
        if (!linea.endsWith(':')) {
            r.valido  = false;
            r.mensaje = "Falta ':' al final de la declaración de bloque.";
            return r;
        }
    }

    // Python no requiere ';' al final de sentencias normales
    if (linea.endsWith(';')) {
        r.valido  = false;
        r.mensaje = "Python no requiere ';' al final de la línea.";
        return r;
    }

    return r;
}
