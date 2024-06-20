#ifndef CATALOGPRODUCTSERIALIZATION_H
#define CATALOGPRODUCTSERIALIZATION_H

#include <QJsonValue>

#include "../models/catalogproductinfo.h"

void parseCatalogProductInfoResponse(const QJsonValue &json, api::GetCatalogProductInfoResponse &data);

#endif // CATALOGPRODUCTSERIALIZATION_H
