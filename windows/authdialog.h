#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>

namespace Ui {
class AuthDialog;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(const QUrl &authUrl, QWidget *parent = nullptr);
    ~AuthDialog();

private slots:
    void on_webEngineView_loadFinished(bool success);

    void on_reloadButton_clicked();

private:
    Ui::AuthDialog *ui;
};

#endif // AUTHDIALOG_H
