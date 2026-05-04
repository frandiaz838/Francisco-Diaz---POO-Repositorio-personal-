#pragma once
#include <QScrollArea>

class ModeloKanban;
class ApiKanban;
class QHBoxLayout;
class QWidget;

// Vista principal del tablero Kanban.
// Muestra todas las columnas en un scroll horizontal.
// Se reconstruye completa cada vez que el modelo cambia.
class KanbanView : public QScrollArea {
    Q_OBJECT
public:
    explicit KanbanView(ModeloKanban* modelo,
                        ApiKanban*    api,
                        QWidget*      parent = nullptr);

private slots:
    void reconstruir();

private:
    ModeloKanban* m_modelo;
    ApiKanban*    m_api;
    QWidget*      m_contenedor;
    QHBoxLayout*  m_hlay;
};
