#include "ValidadorCpp.h"
#include <QRegularExpression>

ResultadoValidacion ValidadorCpp::validarLinea(const QString& lineaOriginal) {
    ResultadoValidacion r;
    QString linea = sinComentario(lineaOriginal, "//").trimmed();
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

    // Lineas que NO requieren punto y coma final
    static const QRegularExpression rxControl(
        R"(^(if|else|for|while|switch|do|case|default|class|struct|namespace|public|private|protected|template|try|catch)\b)");
    static const QRegularExpression rxPreproc(R"(^\s*#)");
    static const QRegularExpression rxEtiqueta(R"(^\s*\w+\s*:\s*$)");
    // Linea que continua en la siguiente: termina en un operador o coma.
    static const QRegularExpression rxContinuacion(
        R"((<<|>>|&&|\|\||->|\+|-|\*|/|%|=|<|>|\?|\.|,|::)$)");

    bool terminaConSimbolo = linea.endsWith(';') || linea.endsWith('{') ||
                             linea.endsWith('}') || linea.endsWith(',') ||
                             linea.endsWith(':') || linea.endsWith('(') ||
                             linea.endsWith('[') || linea.endsWith('\\');

    bool esControl     = rxControl.match(linea).hasMatch();
    bool esPreproc     = rxPreproc.match(linea).hasMatch();
    bool esEtiqueta    = rxEtiqueta.match(linea).hasMatch();
    bool esContinuacion = rxContinuacion.match(linea).hasMatch();

    if (!terminaConSimbolo && !esControl && !esPreproc && !esEtiqueta && !esContinuacion) {
        r.valido  = false;
        r.mensaje = "Falta punto y coma ';' al final de la sentencia.";
        return r;
    }

    // if/for/while requieren parentesis
    static const QRegularExpression rxNecParen(R"(^(if|for|while|switch)\b)");
    if (rxNecParen.match(linea).hasMatch() && !linea.contains('(')) {
        r.valido  = false;
        r.mensaje = "Las estructuras de control requieren paréntesis: " + linea.section(' ', 0, 0) + "(...).";
        return r;
    }

    return r;
}
