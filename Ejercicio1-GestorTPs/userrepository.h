#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QString>

class UserRepository
{
public:
    static QString usersFilePath();

    /// Valida credenciales contra users.json (crea archivo por defecto si no existe).
    static bool validate(const QString &username, const QString &password);
};

#endif
