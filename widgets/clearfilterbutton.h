#ifndef CLEARFILTERBUTTON_H
#define CLEARFILTERBUTTON_H

#include <QWidget>

namespace Ui {
class ClearFilterButton;
}

class ClearFilterButton : public QWidget
{
    Q_OBJECT

public:
    explicit ClearFilterButton(const QString &sectionName,
                               const QString &name,
                               QWidget *parent = nullptr);
    ~ClearFilterButton();

signals:
    void clicked();

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

private:
    Ui::ClearFilterButton *ui;
};

#endif // CLEARFILTERBUTTON_H
