#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyValue.h"

#include "EgoCore/FileName/FileName.h"

ego::rtti::FileNamePropertyValue::FileNamePropertyValue(void* _object, const FileNamePropertyMetaInfo& _propertyMetaInfo)
    : StringPropertyValue(_object, _propertyMetaInfo)
{
}

ego::rtti::FileNamePropertyValue::FileNamePropertyValue(const void* _object, const FileNamePropertyMetaInfo& _propertyMetaInfo)
    : StringPropertyValue(_object, _propertyMetaInfo)
{
}

const ego::rtti::FileNamePropertyMetaInfo& ego::rtti::FileNamePropertyValue::getMetaInfo() const
{
    return static_cast<const FileNamePropertyMetaInfo&>(PropertyValue::getMetaInfo());
}

std::string_view ego::rtti::FileNamePropertyValue::getValue() const
{
    return PropertyValue::getValue<FileName>().getView();
}

bool ego::rtti::FileNamePropertyValue::setValue(std::string_view _string)
{
    FileName* value = PropertyValue::tryGetMutableValue<FileName>();
    if (!value)
    {
        return false;
    }

    *value = _string;

    return true;
}
