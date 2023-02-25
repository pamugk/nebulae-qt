#ifndef CATALOGSERIALIZATION_H
#define CATALOGSERIALIZATION_H

#include <QJsonObject>

#include "../models/catalog.h"

void parseSearchCatalogResponse(const QJsonObject &json, SearchCatalogResponse &data);

#endif // CATALOGSERIALIZATION_H
