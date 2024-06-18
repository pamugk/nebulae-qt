#ifndef LEGALINFODIALOG_H
#define LEGALINFODIALOG_H

#include <QDialog>

namespace Ui {
class LegalInfoDialog;
}

class LegalInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LegalInfoDialog(QWidget *parent = nullptr);
    ~LegalInfoDialog();

private:
    Ui::LegalInfoDialog *ui;
};

#endif // LEGALINFODIALOG_H
