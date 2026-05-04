#include "trabajostore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>

static TrabajoPractico trabajoFromJson(const QJsonObject &o)
{
    TrabajoPractico t;
    t.id = o.value(QStringLiteral("id")).toString();
    t.titulo = o.value(QStringLiteral("titulo")).toString();
    t.estado = o.value(QStringLiteral("estado")).toString();
    t.prioridad = o.value(QStringLiteral("prioridad")).toString();
    t.notas = o.value(QStringLiteral("notas")).toString();
    return t;
}

static QJsonObject trabajoToJson(const TrabajoPractico &t)
{
    QJsonObject o;
    o.insert(QStringLiteral("id"), t.id);
    o.insert(QStringLiteral("titulo"), t.titulo);
    o.insert(QStringLiteral("estado"), t.estado);
    o.insert(QStringLiteral("prioridad"), t.prioridad);
    o.insert(QStringLiteral("notas"), t.notas);
    return o;
}

TrabajoStore::TrabajoStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

bool TrabajoStore::load()
{
    m_items.clear();

    QFile f(m_filePath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return true;

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isArray())
        return false;

    for (const QJsonValue &v : doc.array()) {
        TrabajoPractico t = trabajoFromJson(v.toObject());
        if (t.id.isEmpty())
            t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (t.estado.isEmpty())
            t.estado = QStringLiteral("Pendiente");
        if (t.prioridad.isEmpty())
            t.prioridad = QStringLiteral("Media");
        m_items.append(t);
    }

    return true;
}

bool TrabajoStore::save() const
{
    QJsonArray arr;
    for (const TrabajoPractico &t : m_items)
        arr.append(trabajoToJson(t));

    QFile f(m_filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    return true;
}

void TrabajoStore::add(const TrabajoPractico &t)
{
    m_items.append(t);
}

bool TrabajoStore::update(const TrabajoPractico &t)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == t.id) {
            m_items[i] = t;
            return true;
        }
    }
    return false;
}

bool TrabajoStore::removeById(const QString &id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items.removeAt(i);
            return true;
        }
    }
    return false;
}

TrabajoPractico TrabajoStore::findById(const QString &id) const
{
    for (const TrabajoPractico &t : m_items) {
        if (t.id == id)
            return t;
    }
    return {};
}
