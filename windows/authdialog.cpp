#include "authdialog.h"
#include "ui_authdialog.h"

 #include <QWebEnginePage>

class AuthPage: public QWebEnginePage
{
public:
    AuthPage(QObject* parent): QWebEnginePage(parent) {}

protected:
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
    {
        if (url.host() == "embed.gog.com" && url.path() == "/on_login_success")
        {
            QUrl localUrl("http://127.0.0.1:6543");
            localUrl.setQuery(url.query());
            setUrl(localUrl);
            return false;
        }
        else
        {
            QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
            return true;
        }
    }
};

AuthDialog::AuthDialog(const QUrl &authUrl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog)
{
    ui->setupUi(this);
    connect(ui->webEngineView, &QWebEngineView::loadProgress,
            ui->progressBar, &QProgressBar::setValue);

    // TODO: replace with widget-based implementation
    ui->contentStack->setCurrentWidget(ui->loadingPage);
    auto authPage = new AuthPage(ui->webEngineView);
    ui->webEngineView->setPage(authPage);
    ui->webEngineView->setUrl(authUrl);
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::on_webEngineView_loadFinished(bool success)
{
    if (success)
    {
        ui->contentStack->setCurrentWidget(ui->mainPage);
    }
    else
    {
        ui->contentStack->setCurrentWidget(ui->errorPage);
    }
}


void AuthDialog::on_reloadButton_clicked()
{
    ui->contentStack->setCurrentWidget(ui->loadingPage);
    ui->webEngineView->reload();
}

