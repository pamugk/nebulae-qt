#include <QApplication>
#include <QLocale>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTranslator>

#include "api/gogapiclient.h"
#include "internals/settingsmanager.h"
#include "windows/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QCoreApplication::setOrganizationName("pamugk.github.com");
    QCoreApplication::setApplicationName("Nebulae");

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    foreach (const QString &locale, uiLanguages) {
        const QString baseName = "nebulae_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    SettingsManager settingsManager(&a);

    api::GogApiClient apiClient(&a);
    apiClient.setStoreCredentials(settingsManager.isAutoLogin());
    QObject::connect(&settingsManager, &SettingsManager::autoLoginChanged,
            &apiClient, &api::GogApiClient::setStoreCredentials);

    MainWindow w(&apiClient, &settingsManager);
    w.show();

    QSystemTrayIcon trayIcon(QIcon(":/icons/gog-galaxy.png"), &a);
    QMenu trayMenu;
    QObject::connect(trayMenu.addAction("Show main window"), &QAction::triggered, [&w](bool checked){
        w.show();
    });
    QObject::connect(trayMenu.addAction("Exit"), &QAction::triggered, [&a](bool checked){
        a.exit();
    });
    trayIcon.setContextMenu(&trayMenu);
    trayIcon.setToolTip("Nebulae");
    trayIcon.show();

    return a.exec();
}
