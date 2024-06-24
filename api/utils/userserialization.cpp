#include "userserialization.h"

#include <QJsonArray>

void parseUserData(const QJsonValue &json, api::User &data)
{
    data.id = json["id"].toString();
    data.username = json["username"].toString();
    data.created = QDateTime::fromString(json["created_date"].toString(), Qt::ISODate);
    data.avatar.gogImageId = json["avatar"]["gog_image_id"].toString();
    data.employee = json["is_employee"].toBool();
}

void parseUserSettings(const QJsonValue &json, api::Settings &data)
{
    data.allowToBeInvitedBy = json["allow_to_be_invited_by"].toString();
    data.allowToBeSearched = json["allow_to_be_searched"].toBool();
    data.useTwoStepAuthentication = json["use_two_step_authentication"].toBool();
    data.allowDrmProducts = json["allow_drm_products"].toBool();
}

void parseUserFullData(const QJsonValue &json, api::UserFullData &data)
{
    parseUserData(json, data);
    data.email = json["email"].toString();
    auto tags = json["tags"].toArray();
    data.tags.reserve(tags.size());
    for (const QJsonValue &tag : std::as_const(tags))
    {
        data.tags << tag.toString();
    }
    data.status = json["status"].toInt();
    data.checksum = json["checksum"].toString();
    parseUserSettings(json["settings"], data.settings);
    data.passwordSet = json["password_set"].toBool();
    data.marketingConsent = json["marketing_consent"].toVariant();
}
