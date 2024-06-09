#include "tagutils.h"

QString makeSlugFromText(const QString &text)
{
    return QString(text).toLower().replace(' ', "-");
}
