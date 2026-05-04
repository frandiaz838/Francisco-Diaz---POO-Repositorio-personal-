#ifndef ACTIONLOG_H
#define ACTIONLOG_H

#include <QString>

class ActionLog
{
public:
    static QString logFilePath();

    static void append(const QString &username, const QString &message);
    static QString readAllText();
};

#endif
