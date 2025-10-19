#ifndef STOREHEROBANNER_H
#define STOREHEROBANNER_H

#include <QWidget>

namespace Ui {
class StoreHeroBanner;
}

class StoreHeroBanner : public QWidget
{
    Q_OBJECT

public:
    explicit StoreHeroBanner(QWidget *parent = nullptr);
    ~StoreHeroBanner();

    void setBackgroundImage(const QByteArray &data);
    void setDescription(const QString &description);
    void setEndDateTime(const QDateTime &endDateTime);
    void setPrimaryButtonText(const QString &primaryButtonText);
    void setTitle(const QString &title);
    void setUseDarkTheme(bool useDarkTheme);

signals:
    void primaryButtonClicked();

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private:
    QPixmap backgroundImage;
    Ui::StoreHeroBanner *ui;
};

#endif // STOREHEROBANNER_H
