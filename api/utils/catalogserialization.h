#ifndef CATALOGSERIALIZATION_H
#define CATALOGSERIALIZATION_H

#include <QJsonValue>

#include "../models/catalog.h"

void parseCatalogProduct(const QJsonValue &json, api::CatalogProduct &data, const QString &horizontalCoverFormat);
void parseSearchCatalogResponse(const QJsonValue &json, api::SearchCatalogResponse &data);

#endif // CATALOGSERIALIZATION_H
