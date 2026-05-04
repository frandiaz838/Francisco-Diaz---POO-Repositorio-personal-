#include "MainWindow.h"

#include <QApplication>
#include <QUrl>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CollabDraw"));
    QApplication::setOrganizationName(QStringLiteral("CollabDraw"));

    // Doble clic al .exe: siempre este backend. Override solo si el 1.er arg es http(s)://…
    const QString kDefaultServer = QStringLiteral("http://167.86.77.220:8765");
    QUrl base(kDefaultServer);
    if (argc > 1) {
        const QString a = QString::fromLocal8Bit(argv[1]).trimmed();
        if (a.startsWith(QStringLiteral("http://"), Qt::CaseInsensitive)
            || a.startsWith(QStringLiteral("https://"), Qt::CaseInsensitive)) {
            const QUrl u(a);
            if (u.isValid() && !u.scheme().isEmpty())
                base = u;
        }
    }

    MainWindow w;
    w.startNetworking(base);
    w.show();

    return app.exec();
}
