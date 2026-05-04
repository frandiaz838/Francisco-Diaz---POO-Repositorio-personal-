#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QString>
#include <QDateTime>

class SessionManager
{
public:
    static constexpr int timeoutMs = 5 * 60 * 1000;

    static QString dataDirPath();
    static QString sessionFilePath();

    static bool hasValidSession();
    static QString currentUser();
    static QDateTime sessionStartedAt();

    static void saveSession(const QString &username);
    static void clearSession();
};

#endif
