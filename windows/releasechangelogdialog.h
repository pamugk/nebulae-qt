#ifndef RELEASECHANGELOGDIALOG_H
#define RELEASECHANGELOGDIALOG_H

#include <QDialog>

#include "../api/gogapiclient.h"

namespace Ui {
class ReleaseChangelogDialog;
}

class ReleaseChangelogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReleaseChangelogDialog(const QString &changelog,
                                    const QString &title,
                                    const QString &platform,
                                    const QString &iconLink,
                                    api::GogApiClient *apiClient,
                                    QWidget *parent = nullptr);
    ~ReleaseChangelogDialog();

private:
    Ui::ReleaseChangelogDialog *ui;
};

#endif // RELEASECHANGELOGDIALOG_H
