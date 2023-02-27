#ifndef CATALOGPRODUCTSERIALIZATION_H
#define CATALOGPRODUCTSERIALIZATION_H

#include <QJsonObject>

#include "../models/catalogproductinfo.h"

void parseCatalogProductInfoResponse(const QJsonObject &json, api::GetCatalogProductInfoResponse &data);

#endif // CATALOGPRODUCTSERIALIZATION_H
