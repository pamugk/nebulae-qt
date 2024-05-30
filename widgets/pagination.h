#ifndef PAGINATION_H
#define PAGINATION_H

#include <QPushButton>
#include <QWidget>

namespace Ui {
class Pagination;
}

class Pagination : public QWidget
{
    Q_OBJECT

public:
    explicit Pagination(QWidget *parent = nullptr);
    ~Pagination();

signals:
    void changedPage(quint16 newPage);

public slots:
    void changePages(quint16 page, quint16 totalPages);

private slots:
    void on_firstPageButton_clicked();

private:
    QPushButton *pageButtons[8];
    int pages[8];
    Ui::Pagination *ui;
};

#endif // PAGINATION_H
