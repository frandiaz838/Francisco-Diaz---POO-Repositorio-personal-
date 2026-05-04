#pragma once
#include <QObject>
#include <QJsonArray>
#include <QVector>
#include "Modelos.h"

class ModeloKanban : public QObject {
    Q_OBJECT
public:
    explicit ModeloKanban(QObject* parent = nullptr);

    const QVector<Columna>& columnas() const { return m_columnas; }

public slots:
    void actualizarDesdeJson(const QJsonArray& board);

signals:
    void tableroActualizado();

private:
    QVector<Columna> m_columnas;
};
