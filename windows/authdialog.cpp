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
        } else {
            QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
            return true;
        }
    }
};

AuthDialog::AuthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog)
{
    ui->setupUi(this);

    auto authPage = new AuthPage(ui->webEngineView);
    ui->webEngineView->setPage(authPage);
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::setUrl(const QUrl &authUrl)
{
    ui->webEngineView->setUrl(authUrl);
}
