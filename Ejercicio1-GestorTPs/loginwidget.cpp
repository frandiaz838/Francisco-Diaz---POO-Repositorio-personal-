#include "loginwidget.h"
#include "mainwidget.h"

#include "actionlog.h"
#include "sessionmanager.h"
#include "userrepository.h"

#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    if (SessionManager::hasValidSession()) {
        const QString u = SessionManager::currentUser();
        auto *main = new MainWidget(u);
        main->setAttribute(Qt::WA_DeleteOnClose, false);
        main->show();
        return;
    }

    auto *layout = new QVBoxLayout;

    auto *label = new QLabel(QStringLiteral("Inicio de sesión"));

    user = new QLineEdit;
    user->setPlaceholderText(QStringLiteral("Usuario"));

    pass = new QLineEdit;
    pass->setPlaceholderText(QStringLiteral("Contraseña"));
    pass->setEchoMode(QLineEdit::Password);

    auto *btn = new QPushButton(QStringLiteral("Ingresar"));

    layout->addWidget(label);
    layout->addWidget(user);
    layout->addWidget(pass);
    layout->addWidget(btn);

    setLayout(layout);
    setWindowTitle(QStringLiteral("Planificador — Login"));

    connect(btn, &QPushButton::clicked, this, &LoginWidget::intentarLogin);
}

void LoginWidget::intentarLogin()
{
    const QString u = user->text().trimmed();
    const QString p = pass->text();

    if (!UserRepository::validate(u, p)) {
        ActionLog::append(QStringLiteral("-"),
                            QStringLiteral("LOGIN_FALLIDO usuario=%1").arg(u.isEmpty() ? QStringLiteral("(vacío)") : u));
        QMessageBox::warning(this,
                             QStringLiteral("Login"),
                             QStringLiteral("Usuario o contraseña incorrectos.\n\n"
                                            "Tip: el archivo `data/users.json` se crea automáticamente con `admin` / `1234`."));
        return;
    }

    SessionManager::saveSession(u);
    ActionLog::append(u, QStringLiteral("LOGIN_OK"));

    auto *main = new MainWidget(u);
    main->setAttribute(Qt::WA_DeleteOnClose, false);
    main->show();
    close();
}
