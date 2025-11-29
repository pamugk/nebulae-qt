#ifndef PRESERVATIONSTORYITEM_H
#define PRESERVATIONSTORYITEM_H

#include <QWidget>

namespace Ui {
class PreservationStoryItem;
}

class PreservationStoryItem : public QWidget
{
    Q_OBJECT

public:
    explicit PreservationStoryItem(QWidget *parent = nullptr);
    ~PreservationStoryItem();

    void setCover(const QPixmap &cover);
    void setDescription(const QString &description);
    void setReadFullArticleButtonText(const QString &buttonText);
    void setTitle(const QString &title);

signals:
    void showFullArticleClicked();

private:
    Ui::PreservationStoryItem *ui;
};

#endif // PRESERVATIONSTORYITEM_H
