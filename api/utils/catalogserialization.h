#ifndef CATALOGSERIALIZATION_H
#define CATALOGSERIALIZATION_H

#include <QJsonObject>

#include "../models/catalog.h"

void parseSearchCatalogResponse(const QJsonObject &json, api::SearchCatalogResponse &data);

#endif // CATALOGSERIALIZATION_H
