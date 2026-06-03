#include "ValidadorJava.h"
#include <QRegularExpression>

ResultadoValidacion ValidadorJava::validarLinea(const QString& lineaOriginal) {
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

    // Lineas de bloque o estructura que no requieren ';'
    static const QRegularExpression rxControl(
        R"(^(if|else|for|while|switch|do|case|default|class|interface|public|private|protected|package|import|try|catch|finally)\b)");
    static const QRegularExpression rxAnotacion(R"(^@\w+)");
    static const QRegularExpression rxContinuacion(
        R"((&&|\|\||\+|-|\*|/|%|=|<|>|\?|\.|,)$)");

    bool terminaConSimbolo = linea.endsWith(';') || linea.endsWith('{') ||
                             linea.endsWith('}') || linea.endsWith(',') ||
                             linea.endsWith('(') || linea.endsWith('[');

    bool esControl     = rxControl.match(linea).hasMatch();
    bool esAnotacion   = rxAnotacion.match(linea).hasMatch();
    bool esContinuacion = rxContinuacion.match(linea).hasMatch();

    if (!terminaConSimbolo && !esControl && !esAnotacion && !esContinuacion) {
        r.valido  = false;
        r.mensaje = "Falta punto y coma ';' al final de la sentencia.";
        return r;
    }

    static const QRegularExpression rxNecParen(R"(^(if|for|while|switch)\b)");
    if (rxNecParen.match(linea).hasMatch() && !linea.contains('(')) {
        r.valido  = false;
        r.mensaje = "Las estructuras de control requieren paréntesis.";
        return r;
    }

    return r;
}
