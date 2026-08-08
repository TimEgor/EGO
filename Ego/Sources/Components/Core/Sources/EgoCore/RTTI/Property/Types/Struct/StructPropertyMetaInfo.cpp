#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.h"

ego::rtti::StructPropertyMetaInfo::StructPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const TypeMetaInfo& _valueTypeMetaInfo)
    : PropertyMetaInfo(_name, _offset, _isConst),
      m_valueTypeMetaInfo(_valueTypeMetaInfo)
{
}

const ego::rtti::TypeMetaInfo& ego::rtti::StructPropertyMetaInfo::getValueTypeMetaInfo() const
{
    return m_valueTypeMetaInfo;
}
