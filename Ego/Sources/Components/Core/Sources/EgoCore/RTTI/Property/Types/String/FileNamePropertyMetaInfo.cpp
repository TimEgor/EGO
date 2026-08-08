#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyMetaInfo.h"

#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyValue.h"

ego::rtti::FileNamePropertyMetaInfo::FileNamePropertyMetaInfo(const char* _name, size_t _offset, bool _isConst)
    : StringPropertyMetaInfo(_name, _offset, _isConst)
{
}

ego::rtti::PropertyValuePointer ego::rtti::FileNamePropertyMetaInfo::makePropertyValue(void* _object) const
{
    return MakeIntrusive<FileNamePropertyValue>(_object, *this);
}

ego::rtti::PropertyValuePointer ego::rtti::FileNamePropertyMetaInfo::makePropertyValue(const void* _object) const
{
    return MakeIntrusive<FileNamePropertyValue>(_object, *this);
}
