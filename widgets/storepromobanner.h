#ifndef STOREPROMOBANNER_H
#define STOREPROMOBANNER_H

#include <QWidget>

namespace Ui {
class StorePromoBanner;
}

class StorePromoBanner : public QWidget
{
    Q_OBJECT

public:
    explicit StorePromoBanner(QWidget *parent = nullptr);
    ~StorePromoBanner();

    void setBackgroundImage(const QPixmap &image);
    void setCustomButton(const QString &text);

signals:
    void customInfoClicked();

private:
    Ui::StorePromoBanner *ui;
};

#endif // STOREPROMOBANNER_H
