#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyMetaInfo.h"

#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyValue.h"

ego::rtti::StdStringPropertyMetaInfo::StdStringPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst)
    : StringPropertyMetaInfo(_name, _offset, _isConst)
{
}

ego::rtti::PropertyValuePointer ego::rtti::StdStringPropertyMetaInfo::makePropertyValue(void* _object) const
{
    return MakeIntrusive<StdStringPropertyValue>(_object, *this);
}

ego::rtti::PropertyValuePointer ego::rtti::StdStringPropertyMetaInfo::makePropertyValue(const void* _object) const
{
    return MakeIntrusive<StdStringPropertyValue>(_object, *this);
}
