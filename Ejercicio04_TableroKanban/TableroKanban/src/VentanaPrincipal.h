#pragma once
#include <QLabel>
#include <QMainWindow>
#include <QTimer>

class ModeloKanban;
class ApiKanban;
class KanbanView;

class VentanaPrincipal : public QMainWindow {
    Q_OBJECT
public:
    explicit VentanaPrincipal(const QString& serverUrl,
                               const QString& basicUser,
                               const QString& basicPass,
                               QWidget* parent = nullptr);
    ~VentanaPrincipal() override;

private:
    void aplicarEstilo();

    ModeloKanban* m_modelo;
    ApiKanban*    m_api;
    KanbanView*   m_kanban;
    QLabel*       m_lblEstado;
    QTimer        m_timerPolling;
};
