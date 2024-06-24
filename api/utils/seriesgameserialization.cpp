#include "seriesgameserialization.h"

#include <QJsonArray>

void parseSeriesGame(const QJsonValue &json, api::SeriesGame &data)
{
    if (json["inDevelopment"].isObject())
    {
        data.inDevelopment = json["inDevelopment"]["active"].toBool();
    }
    data.releaseStatus = json["releaseStatus"].toString();

    if (json["_embedded"].isObject())
    {
        auto productData = json["_embedded"];
        if (productData["product"].isObject())
        {
            auto product = productData["product"];
            data.availableForSale = product["isAvailableForSale"].toBool();
            data.visibleInCatalog = product["isVisibleInCatalog"].toBool();
            data.preorder = product["isPreorder"].toBool();
            data.visibleInAccount = product["isVisibleInAccount"].toBool();
            data.id = QString::number(product["id"].toInteger());
            data.title = product["title"].toString();
            data.installable = product["isInstallable"].toBool();
            data.secret = product["isSecret"].toBool();

            if (product["_links"].isObject())
            {
                auto links = product["_links"];
                if (links["image"].isObject())
                {
                    bool templated = links["image"]["templated"].toBool();
                    data.imageLink = templated
                            ? links["image"]["href"].toString().replace("{formatter}", "product_tile_256")
                            : links["image"]["href"].toString();
                }
            }
        }
        data.productType = productData["productType"].toString();
    }
}

void parseGetSeriesGamesResponse(const QJsonValue &json, api::GetSeriesGamesResponse &data)
{
    data.limit = json["limit"].toInt();
    data.page = json["page"].toInt();
    data.pages = json["pages"].toInt();
    auto items = json["_embedded"]["items"].toArray();
    data.items.resize(items.count());
    for (std::size_t i = 0; i < items.count(); i++)
    {
        parseSeriesGame(items[i], data.items[i]);
    }
}
