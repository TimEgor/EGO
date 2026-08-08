#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyValue.h"

ego::rtti::EnumPropertyValue::EnumPropertyValue(void* _object, const EnumPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::EnumPropertyValue::EnumPropertyValue(const void* _object, const EnumPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::EnumPropertyMetaInfo& ego::rtti::EnumPropertyValue::getMetaInfo() const
{
    return static_cast<const EnumPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
