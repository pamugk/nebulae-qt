#ifndef CATALOGSERIALIZATION_H
#define CATALOGSERIALIZATION_H

#include <QJsonObject>

#include "../models/catalog.h"

void parseCatalogProduct(const QJsonObject &json, api::CatalogProduct &data, const QString &horizontalCoverFormat);
void parseSearchCatalogResponse(const QJsonObject &json, api::SearchCatalogResponse &data);

#endif // CATALOGSERIALIZATION_H
