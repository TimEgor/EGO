#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyMetaInfo.h"

#include <string>

ego::rtti::StdStringPropertyMetaInfo::StdStringPropertyMetaInfo(const char* _name, size_t _offset)
    : StringPropertyMetaInfo(_name, _offset)
{
}

const char* ego::rtti::StdStringPropertyMetaInfo::getValue(const void* _value) const
{
    return static_cast<const std::string*>(_value)->c_str();
}

void ego::rtti::StdStringPropertyMetaInfo::setValue(void* _value, const char* _string) const
{
    *static_cast<std::string*>(_value) = _string;
}
