#include "userrepository.h"
#include "sessionmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

QString UserRepository::usersFilePath()
{
    return QDir(SessionManager::dataDirPath()).filePath(QStringLiteral("users.json"));
}

static void ensureDefaultUsersFile()
{
    const QString path = UserRepository::usersFilePath();
    if (QFile::exists(path))
        return;

    QJsonArray arr;
    QJsonObject u;
    u.insert(QStringLiteral("username"), QStringLiteral("admin"));
    u.insert(QStringLiteral("password"), QStringLiteral("1234"));
    arr.append(u);

    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

bool UserRepository::validate(const QString &username, const QString &password)
{
    ensureDefaultUsersFile();

    QFile f(usersFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return false;

    const QJsonArray arr = QJsonDocument::fromJson(f.readAll()).array();
    f.close();

    for (const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        if (o.value(QStringLiteral("username")).toString() == username
            && o.value(QStringLiteral("password")).toString() == password) {
            return true;
        }
    }
    return false;
}
