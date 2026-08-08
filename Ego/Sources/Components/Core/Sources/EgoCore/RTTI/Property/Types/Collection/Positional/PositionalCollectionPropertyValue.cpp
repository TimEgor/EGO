#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyValue.h"

ego::rtti::PositionalCollectionPropertyValue::PositionalCollectionPropertyValue(void* _object, const PositionalCollectionPropertyMetaInfo& _propertyMetaInfo)
    : CollectionPropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::PositionalCollectionPropertyValue::PositionalCollectionPropertyValue(
    const void* _object,
    const PositionalCollectionPropertyMetaInfo& _propertyMetaInfo)
    : CollectionPropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::PositionalCollectionPropertyMetaInfo& ego::rtti::PositionalCollectionPropertyValue::getMetaInfo() const
{
    return static_cast<const PositionalCollectionPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
