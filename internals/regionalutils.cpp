#include "regionalutils.h"

#include <QSet>



QVector<QString> getTerritoryPreferredRatings(QLocale::Territory territory)
{
    switch (territory)
    {
    // Salzburg-Vienna difference not handled?
    case QLocale::Austria:
    case QLocale::Germany:
        return QVector<QString>({"USK", "PEGI"});
    case QLocale::AlandIslands:
    case QLocale::Albania:
    case QLocale::Andorra:
    case QLocale::Anguilla:
    case QLocale::Armenia:
    case QLocale::Azerbaijan:
    case QLocale::Belarus:
    case QLocale::Belgium:
    case QLocale::Bermuda:
    case QLocale::BosniaAndHerzegovina:
    case QLocale::BritishIndianOceanTerritory:
    case QLocale::BritishVirginIslands:
    case QLocale::Bulgaria:
    case QLocale::CanaryIslands:
    case QLocale::CaribbeanNetherlands:
    case QLocale::CaymanIslands:
    case QLocale::CeutaAndMelilla:
    case QLocale::ClippertonIsland:
    case QLocale::Croatia:
    case QLocale::Curacao:
    case QLocale::Cyprus:
    case QLocale::Czechia:
    case QLocale::Denmark:
    case QLocale::DiegoGarcia:
    case QLocale::Estonia:
    case QLocale::EuropeanUnion:
    case QLocale::Europe:
    case QLocale::FalklandIslands:
    case QLocale::FaroeIslands:
    case QLocale::Finland:
    case QLocale::France:
    case QLocale::FrenchGuiana:
    case QLocale::FrenchPolynesia:
    case QLocale::FrenchSouthernTerritories:
    case QLocale::Georgia:
    case QLocale::Gibraltar:
    case QLocale::Greece:
    case QLocale::Greenland:
    case QLocale::Guadeloupe:
    case QLocale::Guernsey:
    case QLocale::Hungary:
    case QLocale::Iceland:
    case QLocale::Ireland:
    case QLocale::IsleOfMan:
    case QLocale::Italy:
    case QLocale::Jersey:
    case QLocale::Kosovo:
    case QLocale::Latvia:
    case QLocale::Liechtenstein:
    case QLocale::Lithuania:
    case QLocale::Luxembourg:
    case QLocale::Macedonia:
    case QLocale::Malta:
    case QLocale::Martinique:
    case QLocale::Mayotte:
    case QLocale::Moldova:
    case QLocale::Monaco:
    case QLocale::Montenegro:
    case QLocale::Netherlands:
    case QLocale::NewCaledonia:
    case QLocale::Norway:
    case QLocale::Pitcairn:
    case QLocale::Poland:
    case QLocale::Portugal:
    case QLocale::Reunion:
    case QLocale::Romania:
    case QLocale::Russia:
    case QLocale::SaintBarthelemy:
    case QLocale::SaintHelena:
    case QLocale::SaintMartin:
    case QLocale::SaintPierreAndMiquelon:
    case QLocale::SanMarino:
    case QLocale::Serbia:
    case QLocale::SintMaarten:
    case QLocale::Slovakia:
    case QLocale::Slovenia:
    case QLocale::SouthGeorgiaAndSouthSandwichIslands:
    case QLocale::Spain:
    case QLocale::SvalbardAndJanMayen:
    case QLocale::Sweden:
    case QLocale::Switzerland:
    case QLocale::TristanDaCunha:
    case QLocale::Turkey:
    case QLocale::TurksAndCaicosIslands:
    case QLocale::Ukraine:
    case QLocale::UnitedKingdom:
    case QLocale::VaticanCity:
    case QLocale::WallisAndFutuna:
        return QVector<QString>({"PEGI"});
    case QLocale::AmericanSamoa:
    case QLocale::Canada:
    case QLocale::Guam:
    case QLocale::MarshallIslands:
    case QLocale::Mexico:
    case QLocale::Micronesia:
    case QLocale::NorthernMarianaIslands:
    case QLocale::Palau:
    case QLocale::UnitedStates:
    case QLocale::UnitedStatesOutlyingIslands:
    case QLocale::UnitedStatesVirginIslands:
    default:
        return QVector<QString>();
    }
}
