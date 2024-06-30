#ifndef DOWNLOADLISTITEM_H
#define DOWNLOADLISTITEM_H

#include <QWidget>

namespace Ui {
class DownloadListItem;
}

class DownloadListItem : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadListItem(QWidget *parent = nullptr);
    ~DownloadListItem();

    void setSize(const QString &value);
    void setTitle(const QString &value);
    void setVersion(const QString &value);

signals:
    void clicked();

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

private:
    Ui::DownloadListItem *ui;
};

#endif // DOWNLOADLISTITEM_H
