#ifndef FEATUREITEM_H
#define FEATUREITEM_H

#include <QMouseEvent>
#include <QWidget>

#include "../api/models/metatag.h"

namespace Ui {
class FeatureItem;
}

class FeatureItem : public QWidget
{
    Q_OBJECT

public:
    explicit FeatureItem(const api::MetaTag &feature, QWidget *parent = nullptr);
    ~FeatureItem();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::FeatureItem *ui;
};

#endif // FEATUREITEM_H
