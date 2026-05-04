#include "sessionmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

QString SessionManager::dataDirPath()
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.mkpath(QStringLiteral("data"));
    return dir.filePath(QStringLiteral("data"));
}

QString SessionManager::sessionFilePath()
{
    return QDir(dataDirPath()).filePath(QStringLiteral("session.json"));
}

bool SessionManager::hasValidSession()
{
    QFile f(sessionFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return false;

    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();

    const QString user = obj.value(QStringLiteral("user")).toString();
    const QString iso = obj.value(QStringLiteral("loginAt")).toString();
    if (user.isEmpty() || iso.isEmpty())
        return false;

    const QDateTime loginAt = QDateTime::fromString(iso, Qt::ISODateWithMs);
    if (!loginAt.isValid())
        return false;

    return loginAt.msecsTo(QDateTime::currentDateTime()) < timeoutMs;
}

QString SessionManager::currentUser()
{
    QFile f(sessionFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return {};

    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    return obj.value(QStringLiteral("user")).toString();
}

QDateTime SessionManager::sessionStartedAt()
{
    QFile f(sessionFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return {};

    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    f.close();
    const QString iso = obj.value(QStringLiteral("loginAt")).toString();
    return QDateTime::fromString(iso, Qt::ISODateWithMs);
}

void SessionManager::saveSession(const QString &username)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("user"), username);
    obj.insert(QStringLiteral("loginAt"), QDateTime::currentDateTime().toString(Qt::ISODateWithMs));

    QFile f(sessionFilePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    f.close();
}

void SessionManager::clearSession()
{
    QFile::remove(sessionFilePath());
}
