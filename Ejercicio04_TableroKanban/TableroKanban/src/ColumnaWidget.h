#pragma once
#include <QFrame>
#include "Modelos.h"

class ApiKanban;
class QVBoxLayout;

// Widget visual de una columna Kanban.
// Contiene el encabezado (nombre + botones) y la lista de TarjetaWidget.
// Habla directamente con ApiKanban y emite operacionCompletada() cuando
// el servidor confirma para que KanbanView haga un fetchBoard.
class ColumnaWidget : public QFrame {
    Q_OBJECT
public:
    // prevColId / nextColId = -1 si no existe columna adyacente
    explicit ColumnaWidget(const Columna& col,
                           int prevColId,
                           int nextColId,
                           ApiKanban* api,
                           QWidget* parent = nullptr);

signals:
    void operacionCompletada();

private:
    void pedirNuevaTarjeta();
    void pedirEditarColumna();
    void pedirEliminarColumna();
    void pedirEditarTarjeta(int cardId);
    void pedirEliminarTarjeta(int cardId);
    void pedirMoverTarjetaIzq(int cardId);
    void pedirMoverTarjetaDer(int cardId);

    Columna    m_col;
    int        m_prevColId;
    int        m_nextColId;
    ApiKanban* m_api;
};
