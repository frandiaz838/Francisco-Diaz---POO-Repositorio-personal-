#ifndef TRABAJOSTORE_H
#define TRABAJOSTORE_H

#include "trabajo.h"

#include <QVector>

class TrabajoStore
{
public:
    explicit TrabajoStore(QString filePath);

    bool load();
    bool save() const;

    QVector<TrabajoPractico> items() const { return m_items; }
    void setItems(const QVector<TrabajoPractico> &items) { m_items = items; }

    void add(const TrabajoPractico &t);
    bool update(const TrabajoPractico &t);
    bool removeById(const QString &id);
    TrabajoPractico findById(const QString &id) const;

private:
    QString m_filePath;
    QVector<TrabajoPractico> m_items;
};

#endif
