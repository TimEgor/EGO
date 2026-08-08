#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

#include "EgoCore/Assert/Assert.h"

ego::rtti::PropertyMetaInfo::PropertyMetaInfo(const char* _name, size_t _offset, bool _isConst)
    : m_name(_name),
      m_offset(_offset),
      m_isConst(_isConst)
{
    EGO_ASSERT(_name && _name[0] != '\0');
}

ego::rtti::PropertyMetaInfo::~PropertyMetaInfo() = default;

const char* ego::rtti::PropertyMetaInfo::getName() const
{
    return m_name;
}

size_t ego::rtti::PropertyMetaInfo::getOffset() const
{
    return m_offset;
}

bool ego::rtti::PropertyMetaInfo::isConst() const
{
    return m_isConst;
}
