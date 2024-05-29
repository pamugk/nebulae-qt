#include "pagination.h"
#include "ui_pagination.h"

#include <algorithm>

Pagination::Pagination(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pagination)
{
    ui->setupUi(this);

    pageButtons[0] = ui->previousPageButton;
    pageButtons[1] = ui->page1Button;
    pageButtons[2] = ui->page2Button;
    pageButtons[3] = ui->page3Button;
    pageButtons[4] = ui->page4Button;
    pageButtons[5] = ui->page5Button;
    pageButtons[6] = ui->lastPageButton;
    pageButtons[7] = ui->nextPageButton;

    for (int i = 0; i < 8; i++)
    {
        connect(pageButtons[i], &QPushButton::clicked, this, [this, i]()
        {
            int requestedPage = pages[i];
            changePages(requestedPage, pages[6]);
            emit changedPage(requestedPage);
        });
    }
}

Pagination::~Pagination()
{
    delete ui;
}

void Pagination::changePages(quint16 page, quint16 totalPages)
{
    ui->previousPageButton->setEnabled(page > 1);
    ui->nextPageButton->setEnabled(page < totalPages);

    if (totalPages < 2)
    {
        this->setVisible(false);
    }
    else
    {
        ui->firstPageButton->setChecked(page == 1);
        ui->lastPageButton->setText(QString::number(totalPages));
        ui->lastPageButton->setChecked(page == totalPages);

        pages[0] = page - 1;
        pages[6] = totalPages;
        pages[7] = page + 1;

        quint8 j = 1;
        for (qint32 i = std::max(std::min(page - 2, totalPages - 5), 2); i < std::min(std::max(page + 3, 7), static_cast<qint32>(totalPages)) && j < 6; i++, j++)
        {
            pages[j] = i;
        }
        for (; j < 6; j++)
        {
            pages[j] = -1;
        }

        for (j = 1; j < 6; j++)
        {
            pageButtons[j]->setVisible(pages[j] > 1 && pages[j] < totalPages);
            pageButtons[j]->setText(QString::number(pages[j]));
            pageButtons[j]->setChecked(pages[j] == page);
        }

        if (pages[1] > 2 && pages[1] + 2 <= page)
        {
            ui->page1Button->setText("…");
        }
        else
        {
            ui->page1Button->setText(QString::number(pages[1]));
        }
        if (page + 2 <= pages[5] && pages[5] < totalPages - 1)
        {
            ui->page5Button->setText("…");
        }
        else
        {
            ui->page5Button->setText(QString::number(pages[5]));
        }

        this->setVisible(true);
    }
}


void Pagination::on_firstPageButton_clicked()
{
    changePages(1, pages[6]);
    emit changedPage(1);
}

