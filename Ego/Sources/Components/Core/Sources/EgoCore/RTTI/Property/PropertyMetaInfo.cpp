#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

#include "EgoCore/Assert/Assert.h"

ego::rtti::PropertyMetaInfo::PropertyMetaInfo(const char* _name, size_t _offset, const TypeMetaInfo* _valueTypeMetaInfo)
    : m_name(_name),
      m_offset(_offset),
      m_valueTypeMetaInfo(_valueTypeMetaInfo)
{
    EGO_ASSERT(_name && _name[0] != '\0');
}

ego::rtti::PropertyMetaInfo::~PropertyMetaInfo() = default;
