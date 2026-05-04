#include "StrokeTypes.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QUuid>

QJsonArray pointsToJson(const QVector<QPointF> &pts)
{
    QJsonArray a;
    for (const QPointF &p : pts) {
        QJsonArray xy;
        xy.append(p.x());
        xy.append(p.y());
        a.append(xy);
    }
    return a;
}

QVector<QPointF> pointsFromJson(const QJsonArray &arr)
{
    QVector<QPointF> pts;
    pts.reserve(arr.size());
    for (const QJsonValue &v : arr) {
        if (!v.isArray())
            continue;
        const QJsonArray xy = v.toArray();
        if (xy.size() < 2)
            continue;
        pts.append(QPointF(xy.at(0).toDouble(), xy.at(1).toDouble()));
    }
    return pts;
}

QJsonObject Stroke::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("id"), id);
    o.insert(QStringLiteral("authorClientId"), authorClientId);
    o.insert(QStringLiteral("isEraser"), isEraser);
    o.insert(QStringLiteral("colorIndex"), colorIndex);
    o.insert(QStringLiteral("widthPx"), widthPx);
    o.insert(QStringLiteral("points"), pointsToJson(points));
    return o;
}

Stroke Stroke::fromJson(const QJsonObject &o)
{
    Stroke s;
    s.id = o.value(QStringLiteral("id")).toString();
    s.authorClientId = o.value(QStringLiteral("authorClientId")).toString();
    s.isEraser = o.value(QStringLiteral("isEraser")).toBool();
    s.colorIndex = o.value(QStringLiteral("colorIndex")).toInt(5);
    s.widthPx = o.value(QStringLiteral("widthPx")).toDouble(4.0);
    s.points = pointsFromJson(o.value(QStringLiteral("points")).toArray());
    return s;
}

Stroke Stroke::createNew(const QString &authorClientId, bool eraser, int colorIdx, double width)
{
    Stroke s;
    s.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.authorClientId = authorClientId;
    s.isEraser = eraser;
    s.colorIndex = qBound(1, colorIdx, 9);
    s.widthPx = width;
    return s;
}
