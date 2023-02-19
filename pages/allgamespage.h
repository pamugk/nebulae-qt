#ifndef ALLGAMESPAGE_H
#define ALLGAMESPAGE_H

#include <QWidget>

namespace Ui {
class AllGamesPage;
}

class AllGamesPage : public QWidget
{
    Q_OBJECT

public:
    explicit AllGamesPage(QWidget *parent = nullptr);
    ~AllGamesPage();

private:
    Ui::AllGamesPage *ui;
};

#endif // ALLGAMESPAGE_H
