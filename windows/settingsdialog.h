#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void on_generalButton_clicked();

    void on_interfaceButton_clicked();

    void on_notificationsButton_clicked();

    void on_installingButton_clicked();

    void on_featuresButton_clicked();

private:
    Ui::SettingsDialog *ui;
    QPushButton *selectedSection;
};

#endif // SETTINGSDIALOG_H
