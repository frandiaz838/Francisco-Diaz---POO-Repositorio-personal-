#pragma once
#include <QHash>
#include <QPlainTextEdit>
#include "ValidadorSintaxis.h"

// Subclase de QPlainTextEdit con validacion de sintaxis por linea.
// Trabaja con un puntero a la clase base abstracta ValidadorSintaxis,
// demostrando polimorfismo: el editor no sabe si valida C++, Python o Java.
class EditorTexto : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit EditorTexto(QWidget* parent = nullptr);

    void setValidador(ValidadorSintaxis* v);
    QString lenguajeActual() const;

    // Valida una linea por su numero (0-based) y aplica resaltado.
    void validarLineaNumero(int numeroLinea);
    void revalidarTodo();

signals:
    void validacionLinea(int numeroLinea, bool valida, const QString& mensaje);

protected:
    void focusOutEvent(QFocusEvent* ev) override;
    void focusInEvent(QFocusEvent* ev) override;
    void keyPressEvent(QKeyEvent* ev) override;

private slots:
    void onCursorMovido();

private:
    void aplicarResaltado();

    ValidadorSintaxis* m_validador = nullptr;
    int                m_lineaPrevia = 0;
    // numero de linea -> mensaje (las que tienen entrada estan en rojo)
    QHash<int, QString> m_lineasInvalidas;
};
