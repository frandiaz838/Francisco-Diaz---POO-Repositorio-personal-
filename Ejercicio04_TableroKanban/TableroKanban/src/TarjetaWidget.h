#pragma once
#include <QFrame>
#include "Modelos.h"

// Widget visual de una tarjeta Kanban.
// Muestra titulo, descripcion y botones de accion.
class TarjetaWidget : public QFrame {
    Q_OBJECT
public:
    explicit TarjetaWidget(const Tarjeta& t,
                           bool puedeIrIzq,
                           bool puedeIrDer,
                           QWidget* parent = nullptr);

signals:
    void editarPedido(int id);
    void eliminarPedido(int id);
    void moverIzquierda(int id);
    void moverDerecha(int id);

private:
    int m_id;
};
