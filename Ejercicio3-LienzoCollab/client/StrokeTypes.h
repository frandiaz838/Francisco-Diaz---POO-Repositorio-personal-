#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVector>

struct Stroke {
    QString id;
    QString authorClientId;
    bool isEraser = false;
    int colorIndex = 5;
    double widthPx = 4.0;
    QVector<QPointF> points;

    QJsonObject toJson() const;
    static Stroke fromJson(const QJsonObject &o);
    static Stroke createNew(const QString &authorClientId, bool eraser, int colorIdx, double width);
};

QJsonArray pointsToJson(const QVector<QPointF> &pts);
QVector<QPointF> pointsFromJson(const QJsonArray &arr);
