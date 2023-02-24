#include "allgamespage.h"
#include "ui_allgamespage.h"

#include "../widgets/collapsiblearea.h"
#include "../widgets/filtercheckbox.h"

AllGamesPage::AllGamesPage(QWidget *parent) :
    BasePage(parent),
    ui(new Ui::AllGamesPage)
{
    ui->setupUi(this);

    auto priceArea = new CollapsibleArea("Price range", ui->filtersScrollAreaContents);
    auto priceAreaLayout = new QVBoxLayout(priceArea);
    priceAreaLayout->addWidget(new QCheckBox("Show only free games", priceArea));
    priceArea->setContentLayout(priceAreaLayout);

    auto releaseStatusArea = new CollapsibleArea("Release Status", ui->filtersScrollAreaContents);
    auto releaseStatusAreaLayout = new QVBoxLayout(releaseStatusArea);
    releaseStatusAreaLayout->addWidget(new FilterCheckbox("New arrivals", releaseStatusArea));
    releaseStatusAreaLayout->addWidget(new FilterCheckbox("Upcoming", releaseStatusArea));
    releaseStatusAreaLayout->addWidget(new FilterCheckbox("Early access", releaseStatusArea));
    releaseStatusArea->setContentLayout(releaseStatusAreaLayout);

    auto genresArea = new CollapsibleArea("Genres", ui->filtersScrollAreaContents);
    auto genresAreaLayout = new QVBoxLayout(genresArea);
    genresAreaLayout->addWidget(new FilterCheckbox("Action", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Adventure", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Racing", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Role-playing", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Shooter", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Simulation", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Sports", genresArea));
    genresAreaLayout->addWidget(new FilterCheckbox("Strategy", genresArea));
    genresArea->setContentLayout(genresAreaLayout);

    auto tagsArea = new CollapsibleArea("Tags", ui->filtersScrollAreaContents);
    auto tagsAreaLayout = new QVBoxLayout(tagsArea);
    tagsAreaLayout->addWidget(new FilterCheckbox("Indie", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Fantasy", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Story Rich", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Atmospheric", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("2D", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Science", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Classic", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Turn-Based", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Puzzle", tagsArea));
    tagsAreaLayout->addWidget(new FilterCheckbox("Great Soundtrack", tagsArea));
    tagsArea->setContentLayout(tagsAreaLayout);

    auto osArea = new CollapsibleArea("Operating Systems", ui->filtersScrollAreaContents);
    auto osAreaLayout = new QVBoxLayout(osArea);
    osAreaLayout->addWidget(new FilterCheckbox("Windows", osArea));
    osAreaLayout->addWidget(new FilterCheckbox("Mac OS", osArea));
    osAreaLayout->addWidget(new FilterCheckbox("Linux", osArea));
    osArea->setContentLayout(osAreaLayout);

    auto featuresArea = new CollapsibleArea("Features", ui->filtersScrollAreaContents);
    auto featuresAreaLayout = new QVBoxLayout(featuresArea);
    featuresAreaLayout->addWidget(new FilterCheckbox("Single-player", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Multi-player", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Co-op", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Achievements", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Leaderboards", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Controller support", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Cloud saves", featuresArea));
    featuresAreaLayout->addWidget(new FilterCheckbox("Overlay", featuresArea));
    featuresArea->setContentLayout(featuresAreaLayout);

    auto releaseDateArea = new CollapsibleArea("Release Date", ui->filtersScrollAreaContents);

    auto languagesArea = new CollapsibleArea("Languages", ui->filtersScrollAreaContents);

    ui->filtersScrollAreaLayout->setAlignment(Qt::AlignTop);
    ui->filtersScrollAreaLayout->addWidget(priceArea);
    ui->filtersScrollAreaLayout->addWidget(releaseStatusArea);
    ui->filtersScrollAreaLayout->addWidget(genresArea);
    ui->filtersScrollAreaLayout->addWidget(tagsArea);
    ui->filtersScrollAreaLayout->addWidget(osArea);
    ui->filtersScrollAreaLayout->addWidget(featuresArea);
    ui->filtersScrollAreaLayout->addWidget(releaseDateArea);
    ui->filtersScrollAreaLayout->addWidget(languagesArea);
}

AllGamesPage::~AllGamesPage()
{
    delete ui;
}

void AllGamesPage::setApiClient(GogApiClient *apiClient)
{

}

void AllGamesPage::clear()
{

}

void AllGamesPage::initialize()
{

}
