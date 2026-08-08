#include "EgoCore/RTTI/Property/Types/String/StringPropertyValue.h"

ego::rtti::StringPropertyValue::StringPropertyValue(void* _object, const StringPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::StringPropertyValue::StringPropertyValue(const void* _object, const StringPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::StringPropertyMetaInfo& ego::rtti::StringPropertyValue::getMetaInfo() const
{
    return static_cast<const StringPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
