#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"

#include "EgoCore/Assert/Assert.h"

ego::rtti::PropertyMetaInfo& ego::rtti::CastPropertyMetaInfo(PropertyMetaInfo& _propertyMetaInfo, const TypeMetaInfo& _targetTypeMetaInfo)
{
    EGO_ASSERT(_propertyMetaInfo.getObjectTypeMetaInfo().isBasedOn(_targetTypeMetaInfo));

    return _propertyMetaInfo;
}

const ego::rtti::PropertyMetaInfo& ego::rtti::CastPropertyMetaInfo(const PropertyMetaInfo& _propertyMetaInfo, const TypeMetaInfo& _targetTypeMetaInfo)
{
    EGO_ASSERT(_propertyMetaInfo.getObjectTypeMetaInfo().isBasedOn(_targetTypeMetaInfo));

    return _propertyMetaInfo;
}
