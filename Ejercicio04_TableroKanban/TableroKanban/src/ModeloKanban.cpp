#include "ModeloKanban.h"
#include <QJsonObject>

ModeloKanban::ModeloKanban(QObject* parent) : QObject(parent) {}

void ModeloKanban::actualizarDesdeJson(const QJsonArray& board) {
    m_columnas.clear();
    for (const QJsonValue& v : board) {
        if (!v.isObject()) continue;
        QJsonObject obj = v.toObject();

        Columna col;
        col.id       = obj["id"].toInt();
        col.nombre   = obj["name"].toString();
        col.posicion = obj["position"].toInt();

        for (const QJsonValue& cv : obj["cards"].toArray()) {
            if (!cv.isObject()) continue;
            QJsonObject co = cv.toObject();
            Tarjeta t;
            t.id          = co["id"].toInt();
            t.columnId    = co["column_id"].toInt();
            t.titulo      = co["title"].toString();
            t.descripcion = co["description"].toString();
            t.posicion    = co["position"].toInt();
            col.tarjetas.append(t);
        }
        m_columnas.append(col);
    }
    emit tableroActualizado();
}
