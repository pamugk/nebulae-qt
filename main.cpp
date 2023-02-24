#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "internals/settingsmanager.h"
#include "windows/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("pamugk.github.com");
    QCoreApplication::setApplicationName("GOG Galaxy (Unofficial)");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "unofficial-gog-client_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    SettingsManager settingsManager;

    MainWindow w;
    w.setSettingsManager(&settingsManager);
    w.show();
    return a.exec();
}
