#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyValue.h"

ego::rtti::ScalarPropertyValue::ScalarPropertyValue(void* _object, const ScalarPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::ScalarPropertyValue::ScalarPropertyValue(const void* _object, const ScalarPropertyMetaInfo& _propertyMetaInfo)
    : PropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::ScalarPropertyMetaInfo& ego::rtti::ScalarPropertyValue::getMetaInfo() const
{
    return static_cast<const ScalarPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}
