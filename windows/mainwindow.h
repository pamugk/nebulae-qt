#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

#include "../api/gogapiclient.h"
#include "../internals/navigationdestination.h"
#include "../internals/settingsmanager.h"
#include "../pages/basepage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setApiClient(api::GogApiClient *apiClient);
    void setSettingsManager(SettingsManager *settingsManager);

public slots:
    void setDestination(NavigationDestination destination);

private slots:
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

private:
    api::GogApiClient *apiClient;
    SettingsManager *settingsManager;

    Ui::MainWindow *ui;
    Page currentPage;
    QMap<Page, BasePage *> pages;

    void navigate(Page newPage, const QVariant &data = QVariant());
};
#endif // MAINWINDOW_H
