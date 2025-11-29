#ifndef PRESERVATIONPROGRAMSERIALIZATION_H
#define PRESERVATIONPROGRAMSERIALIZATION_H

#include <QJsonValue>

#include "../models/preservationprogram.h"

void parseGetAccordionSectionResponse(const QJsonValue &json, api::GetAccordionSectionResponse &data);
void parseGetPreservationProgramDescriptionSectionResponse(const QJsonValue &json, api::GetPreservationProgramDescriptionSectionResponse &data);
void parseGetPreservationProgramHeroSectionResponse(const QJsonValue &json, api::GetPreservationProgramHeroSectionResponse &data);
void parseGetPreservationProgramStoriesSectionResponse(const QJsonValue &json, api::GetPreservationProgramStoriesSectionResponse &data);

#endif // PRESERVATIONPROGRAMSERIALIZATION_H
