#ifndef NEWSITEMTILE_H
#define NEWSITEMTILE_H

#include <QWidget>

#include "../api/gogapiclient.h"
#include "../api/models/news.h"

namespace Ui {
class NewsItemTile;
}

class NewsItemTile : public QWidget
{
    Q_OBJECT

public:
    explicit NewsItemTile(const api::NewsItem &data,
                          bool primary,
                          api::GogApiClient *apiClient,
                          QWidget *parent = nullptr);
    ~NewsItemTile();

signals:
    void navigateToNewsItem(unsigned long long id);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    unsigned long long itemId;
    QNetworkReply *imageReply;
    Ui::NewsItemTile *ui;
};

#endif // NEWSITEMTILE_H
