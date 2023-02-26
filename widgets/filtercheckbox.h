#ifndef FILTERCHECKBOX_H
#define FILTERCHECKBOX_H

#include <QWidget>

namespace Ui {
class FilterCheckbox;
}

class FilterCheckbox : public QWidget
{
    Q_OBJECT

public:
    explicit FilterCheckbox(const QString &text, QWidget *parent = nullptr);
    ~FilterCheckbox();

signals:
    void include(bool value);
    void exclude(bool value);

private slots:
    void on_hideButton_toggled(bool checked);

private:
    Ui::FilterCheckbox *ui;
};

#endif // FILTERCHECKBOX_H
