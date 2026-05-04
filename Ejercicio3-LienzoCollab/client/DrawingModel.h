#pragma once

#include "StrokeTypes.h"

#include <QHash>
#include <QJsonArray>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

class DrawingModel : public QObject
{
    Q_OBJECT
public:
    explicit DrawingModel(QObject *parent = nullptr);

    const QVector<Stroke> &strokes() const { return m_strokes; }
    qint64 serverRevision() const { return m_serverRevision; }
    void setServerRevision(qint64 r);

    bool hasStrokeId(const QString &id) const;

    void addStrokeFromLocal(const Stroke &stroke);
    void ingestRemoteStroke(const Stroke &stroke);

    QList<Stroke> pendingStrokes() const;
    void markStrokesSynced(const QStringList &ids);

    void applyFullServerState(const QJsonArray &serverStrokes, qint64 revision);

    QJsonArray allStrokesJson() const;
    QJsonArray pendingStrokesJson() const;

signals:
    void modelReset();
    void strokeAdded(int index);

private:
    void appendStroke(const Stroke &stroke, bool pending);
    void rebuildIdIndex();

    QVector<Stroke> m_strokes;
    QHash<QString, int> m_idToIndex;
    QSet<QString> m_pendingIds;
    qint64 m_serverRevision = 0;
};
