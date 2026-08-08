#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyValue.h"

ego::rtti::CollectionPropertyValue::CollectionPropertyValue(void* _object, const CollectionPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::CollectionPropertyValue::CollectionPropertyValue(const void* _object, const CollectionPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::CollectionPropertyMetaInfo& ego::rtti::CollectionPropertyValue::getMetaInfo() const
{
    return static_cast<const CollectionPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
