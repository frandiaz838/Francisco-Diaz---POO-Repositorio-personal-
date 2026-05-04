#include "DrawingModel.h"

DrawingModel::DrawingModel(QObject *parent) : QObject(parent) {}

void DrawingModel::setServerRevision(qint64 r)
{
    m_serverRevision = r;
}

bool DrawingModel::hasStrokeId(const QString &id) const
{
    return m_idToIndex.contains(id);
}

void DrawingModel::rebuildIdIndex()
{
    m_idToIndex.clear();
    for (int i = 0; i < m_strokes.size(); ++i)
        m_idToIndex.insert(m_strokes.at(i).id, i);
}

void DrawingModel::appendStroke(const Stroke &stroke, bool pending)
{
    m_strokes.append(stroke);
    const int idx = m_strokes.size() - 1;
    m_idToIndex.insert(stroke.id, idx);
    if (pending)
        m_pendingIds.insert(stroke.id);
    emit strokeAdded(idx);
}

void DrawingModel::addStrokeFromLocal(const Stroke &stroke)
{
    if (m_idToIndex.contains(stroke.id))
        return;
    appendStroke(stroke, true);
}

void DrawingModel::ingestRemoteStroke(const Stroke &stroke)
{
    if (stroke.id.isEmpty() || m_idToIndex.contains(stroke.id))
        return;
    appendStroke(stroke, false);
}

QList<Stroke> DrawingModel::pendingStrokes() const
{
    QList<Stroke> out;
    for (const Stroke &s : m_strokes) {
        if (m_pendingIds.contains(s.id))
            out.append(s);
    }
    return out;
}

void DrawingModel::markStrokesSynced(const QStringList &ids)
{
    for (const QString &id : ids)
        m_pendingIds.remove(id);
}

void DrawingModel::applyFullServerState(const QJsonArray &serverStrokes, qint64 revision)
{
    QVector<Stroke> pendingLocal;
    for (const Stroke &s : m_strokes) {
        if (m_pendingIds.contains(s.id))
            pendingLocal.append(s);
    }

    QSet<QString> serverIds;
    QVector<Stroke> next;
    next.reserve(serverStrokes.size() + pendingLocal.size());
    for (const QJsonValue &v : serverStrokes) {
        if (!v.isObject())
            continue;
        Stroke s = Stroke::fromJson(v.toObject());
        if (s.id.isEmpty())
            continue;
        serverIds.insert(s.id);
        next.append(s);
    }
    for (const Stroke &s : pendingLocal) {
        if (!serverIds.contains(s.id))
            next.append(s);
    }

    m_strokes = next;
    m_pendingIds.clear();
    for (const Stroke &s : m_strokes) {
        if (!serverIds.contains(s.id))
            m_pendingIds.insert(s.id);
    }

    m_serverRevision = revision;
    rebuildIdIndex();
    emit modelReset();
}

QJsonArray DrawingModel::allStrokesJson() const
{
    QJsonArray a;
    for (const Stroke &s : m_strokes)
        a.append(s.toJson());
    return a;
}

QJsonArray DrawingModel::pendingStrokesJson() const
{
    QJsonArray a;
    for (const Stroke &s : m_strokes) {
        if (m_pendingIds.contains(s.id))
            a.append(s.toJson());
    }
    return a;
}
