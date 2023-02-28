#ifndef CATALOGPRODUCTPAGE_H
#define CATALOGPRODUCTPAGE_H

#include "./basepage.h"

namespace Ui {
class CatalogProductPage;
}

class CatalogProductPage : public BasePage
{
    Q_OBJECT

public:
    explicit CatalogProductPage(QWidget *parent = nullptr);
    ~CatalogProductPage();

    virtual void setApiClient(api::GogApiClient *apiClient) override;

public slots:
    virtual void clear() override;
    virtual void initialize() override;
    void setProductId(quint64 id);

private slots:
    void on_acceptContentWarningButton_clicked();

    void on_goBackButton_clicked();

    void descriptionViewContentsSizeChanged(const QSizeF &size);

private:
    api::GogApiClient *apiClient;
    quint64 id;
    Ui::CatalogProductPage *ui;
};

#endif // CATALOGPRODUCTPAGE_H
