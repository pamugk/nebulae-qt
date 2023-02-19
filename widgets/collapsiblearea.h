#ifndef COLLAPSIBLEAREA_H
#define COLLAPSIBLEAREA_H

#include <QParallelAnimationGroup>
#include <QWidget>

namespace Ui {
class CollapsibleArea;
}

class CollapsibleArea : public QWidget
{
    Q_OBJECT

public:
    explicit CollapsibleArea(const QString &header, QWidget *parent = nullptr);
    ~CollapsibleArea();
    void setContentLayout(QLayout *contentLayout);

private slots:
    void on_expandButton_toggled(bool checked);

private:
    Ui::CollapsibleArea *ui;
    QParallelAnimationGroup toggleAnimation;
};

#endif // COLLAPSIBLEAREA_H
