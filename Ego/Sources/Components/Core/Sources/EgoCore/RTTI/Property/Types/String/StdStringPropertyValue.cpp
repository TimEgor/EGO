#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyValue.h"

#include <string>

ego::rtti::StdStringPropertyValue::StdStringPropertyValue(void* _object, const StdStringPropertyMetaInfo& _propertyMetaInfo)
    : StringPropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::StdStringPropertyValue::StdStringPropertyValue(const void* _object, const StdStringPropertyMetaInfo& _propertyMetaInfo)
    : StringPropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::StdStringPropertyMetaInfo& ego::rtti::StdStringPropertyValue::getMetaInfo() const
{
    return static_cast<const StdStringPropertyMetaInfo&>(PropertyValue::getMetaInfo());
}

std::string_view ego::rtti::StdStringPropertyValue::getValue() const
{
    return PropertyValue::getValue<std::string>();
}

bool ego::rtti::StdStringPropertyValue::setValue(std::string_view _string)
{
    std::string* value = PropertyValue::tryGetMutableValue<std::string>();
    if (!value)
    {
        return false;
    }

    *value = _string;

    return true;
}
