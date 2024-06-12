#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QStack>

#include "../api/gogapiclient.h"
#include "../internals/navigationdestination.h"
#include "../internals/settingsmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(api::GogApiClient *, SettingsManager *settingsManager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigate(NavigationDestination destination);
    void navigateBack();
    void navigateForward();

    void on_discoverButton_clicked();

    void on_recentButton_clicked();

    void on_storeButton_clicked();

    void on_allGamesButton_clicked();

    void on_wishlistButton_clicked();

    void on_ordersButton_clicked();

    void on_libraryButton_clicked();

    void on_installedButton_clicked();

    void on_friendsButton_clicked();

    void on_settingsButton_clicked();

    void on_actionButton_clicked();

    void on_navigateBackButton_clicked();

    void on_navigateForwardButton_clicked();

    void on_logoutButton_clicked();

private:
    api::GogApiClient *apiClient;
    QStack<NavigationDestination> navigationHistory;
    QStack<NavigationDestination> navigationHistoryReplay;
    SettingsManager *settingsManager;

    Ui::MainWindow *ui;

    QWidget *initializePage(const NavigationDestination &destination);
    void switchUiAuthenticatedState(bool authenticated);
    void updateCheckedDrawerDestination(Page currentPage);
};
#endif // MAINWINDOW_H
