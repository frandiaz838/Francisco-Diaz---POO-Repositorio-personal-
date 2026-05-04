#include "actionlog.h"
#include "sessionmanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>

QString ActionLog::logFilePath()
{
    return QDir(SessionManager::dataDirPath()).filePath(QStringLiteral("historial.log"));
}

void ActionLog::append(const QString &username, const QString &message)
{
    const QString line = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
        + QStringLiteral(" | ")
        + (username.isEmpty() ? QStringLiteral("-") : username)
        + QStringLiteral(" | ")
        + message
        + QLatin1Char('\n');

    QFile f(logFilePath());
    f.open(QIODevice::WriteOnly | QIODevice::Append);
    f.write(line.toUtf8());
}

QString ActionLog::readAllText()
{
    QFile f(logFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(f.readAll());
}
