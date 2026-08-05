#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyMetaInfo.h"

#include "EgoCore/FileName/FileName.h"

ego::rtti::FileNamePropertyMetaInfo::FileNamePropertyMetaInfo(const char* _name, size_t _offset)
    : StringPropertyMetaInfo(_name, _offset)
{
}

const char* ego::rtti::FileNamePropertyMetaInfo::getValue(const void* _value) const
{
    return static_cast<const FileName*>(_value)->c_str();
}

void ego::rtti::FileNamePropertyMetaInfo::setValue(void* _value, const char* _string) const
{
    *static_cast<FileName*>(_value) = _string;
}
