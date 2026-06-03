#include "EditorTexto.h"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QTextBlock>
#include <QTextCursor>

EditorTexto::EditorTexto(QWidget* parent) : QPlainTextEdit(parent) {
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(32);
    QFont mono("Consolas");
    mono.setStyleHint(QFont::Monospace);
    mono.setPointSize(11);
    setFont(mono);
    setStyleSheet(
        "QPlainTextEdit { background: #1e1e2e; color: #e4e4e7; "
        "border: 1px solid rgba(74,144,226,80); border-radius: 6px; padding: 8px; "
        "selection-background-color: #4A90E2; }"
    );

    // Signal/slot: cuando el cursor se mueve, detectamos si cambio de linea
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &EditorTexto::onCursorMovido);
}

void EditorTexto::setValidador(ValidadorSintaxis* v) {
    m_validador = v;
    m_lineasInvalidas.clear();
    revalidarTodo();
}

QString EditorTexto::lenguajeActual() const {
    return m_validador ? m_validador->nombreLenguaje() : QString();
}

void EditorTexto::onCursorMovido() {
    int lineaActual = textCursor().blockNumber();
    if (lineaActual != m_lineaPrevia) {
        // El usuario abandono la linea previa: la validamos ahora.
        validarLineaNumero(m_lineaPrevia);
        m_lineaPrevia = lineaActual;
    }
}

void EditorTexto::validarLineaNumero(int numeroLinea) {
    if (!m_validador) return;
    QTextBlock bloque = document()->findBlockByNumber(numeroLinea);
    if (!bloque.isValid()) return;

    ResultadoValidacion r = m_validador->validarLinea(bloque.text());
    if (r.valido) {
        m_lineasInvalidas.remove(numeroLinea);
    } else {
        m_lineasInvalidas[numeroLinea] = r.mensaje;
    }
    aplicarResaltado();
    emit validacionLinea(numeroLinea, r.valido, r.mensaje);
}

void EditorTexto::revalidarTodo() {
    if (!m_validador) return;
    m_lineasInvalidas.clear();
    int total = document()->blockCount();
    for (int i = 0; i < total; ++i) {
        QTextBlock b = document()->findBlockByNumber(i);
        ResultadoValidacion r = m_validador->validarLinea(b.text());
        if (!r.valido)
            m_lineasInvalidas[i] = r.mensaje;
    }
    aplicarResaltado();
}

void EditorTexto::aplicarResaltado() {
    QList<QTextEdit::ExtraSelection> selecciones;
    for (auto it = m_lineasInvalidas.constBegin();
         it != m_lineasInvalidas.constEnd(); ++it) {
        QTextBlock b = document()->findBlockByNumber(it.key());
        if (!b.isValid()) continue;

        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor(220, 60, 60, 90));
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = QTextCursor(b);
        sel.cursor.clearSelection();
        selecciones.append(sel);
    }
    setExtraSelections(selecciones);
}

void EditorTexto::focusOutEvent(QFocusEvent* ev) {
    // Al perder foco validamos la linea actual (disparo de validacion).
    validarLineaNumero(textCursor().blockNumber());
    QPlainTextEdit::focusOutEvent(ev);
}

void EditorTexto::focusInEvent(QFocusEvent* ev) {
    QPlainTextEdit::focusInEvent(ev);
}

void EditorTexto::keyPressEvent(QKeyEvent* ev) {
    // Enter/Return: valida la linea que se esta abandonando
    if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
        int lineaAVerificar = textCursor().blockNumber();
        QPlainTextEdit::keyPressEvent(ev);
        validarLineaNumero(lineaAVerificar);
        m_lineaPrevia = textCursor().blockNumber();
        return;
    }
    QPlainTextEdit::keyPressEvent(ev);
}
