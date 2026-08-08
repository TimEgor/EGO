#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyValue.h"

ego::rtti::AssociativeCollectionPropertyValue::AssociativeCollectionPropertyValue(void* _object, const AssociativeCollectionPropertyMetaInfo& _propertyMetaInfo)
    : CollectionPropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::AssociativeCollectionPropertyValue::AssociativeCollectionPropertyValue(
    const void* _object,
    const AssociativeCollectionPropertyMetaInfo& _propertyMetaInfo)
    : CollectionPropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::AssociativeCollectionPropertyMetaInfo& ego::rtti::AssociativeCollectionPropertyValue::getMetaInfo() const
{
    return static_cast<const AssociativeCollectionPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
