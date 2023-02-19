#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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

private:
    Ui::MainWindow *ui;
    QWidget *storePage;
    QWidget *allGamesPage;
    QWidget *wishlistPage;
    QWidget *ordersPage;
};
#endif // MAINWINDOW_H
