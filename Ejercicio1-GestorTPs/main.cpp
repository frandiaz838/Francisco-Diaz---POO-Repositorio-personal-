#include "loginwidget.h"
#include "sessionmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("PlanificadorTP"));

    LoginWidget w;
    if (!SessionManager::hasValidSession())
        w.show();

    return a.exec();
}
