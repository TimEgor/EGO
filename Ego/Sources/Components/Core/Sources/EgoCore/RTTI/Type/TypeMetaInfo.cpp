#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

#include <cstdint>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

ego::rtti::TypeMetaInfo::ParentTypeMetaInfoContext::ParentTypeMetaInfoContext() = default;

ego::rtti::TypeMetaInfo::ParentTypeMetaInfoContext::ParentTypeMetaInfoContext(const TypeMetaInfo* _info, size_t _offset)
    : m_info(_info),
      m_offset(_offset)
{
}

ego::rtti::TypeMetaInfo::PropertyIterator::PropertyIterator(const TypeMetaInfo* _typeMetaInfo)
    : m_typeMetaInfo(_typeMetaInfo)
{
    advanceToProperty();
}

const ego::rtti::PropertyMetaInfo& ego::rtti::TypeMetaInfo::PropertyIterator::operator*() const
{
    EGO_ASSERT(m_typeMetaInfo);

    const TypeMetaInfo* typeMetaInfo = m_typeMetaInfo;
    if (m_typeIndex > 0)
    {
        typeMetaInfo = m_typeMetaInfo->m_parentTypeMetaInfos[m_typeIndex - 1].m_info;
    }

    EGO_ASSERT(typeMetaInfo);
    EGO_ASSERT(m_propertyIndex < typeMetaInfo->m_properties.size());

    const PropertyMetaInfoPointer& propertyMetaInfo = typeMetaInfo->m_properties[m_propertyIndex];
    EGO_ASSERT(propertyMetaInfo);

    return *propertyMetaInfo;
}

ego::rtti::TypeMetaInfo::PropertyIterator& ego::rtti::TypeMetaInfo::PropertyIterator::operator++()
{
    EGO_ASSERT(m_typeMetaInfo);

    ++m_propertyIndex;
    advanceToProperty();

    return *this;
}

ego::rtti::TypeMetaInfo::PropertyIterator ego::rtti::TypeMetaInfo::PropertyIterator::operator++(int)
{
    PropertyIterator iterator = *this;
    ++(*this);

    return iterator;
}

bool ego::rtti::TypeMetaInfo::PropertyIterator::operator==(const PropertyIterator& _iterator) const
{
    return m_typeMetaInfo == _iterator.m_typeMetaInfo && m_typeIndex == _iterator.m_typeIndex && m_propertyIndex == _iterator.m_propertyIndex;
}

bool ego::rtti::TypeMetaInfo::PropertyIterator::operator!=(const PropertyIterator& _iterator) const
{
    return !(*this == _iterator);
}

void* ego::rtti::TypeMetaInfo::PropertyIterator::getValueAddress(void* _object) const
{
    EGO_ASSERT(_object);

    return static_cast<uint8_t*>(_object) + getValueOffset();
}

const void* ego::rtti::TypeMetaInfo::PropertyIterator::getValueAddress(const void* _object) const
{
    EGO_ASSERT(_object);

    return static_cast<const uint8_t*>(_object) + getValueOffset();
}

void ego::rtti::TypeMetaInfo::PropertyIterator::advanceToProperty()
{
    while (m_typeMetaInfo)
    {
        const TypeMetaInfo* typeMetaInfo = m_typeMetaInfo;
        if (m_typeIndex > 0)
        {
            typeMetaInfo = m_typeMetaInfo->m_parentTypeMetaInfos[m_typeIndex - 1].m_info;
        }

        EGO_ASSERT(typeMetaInfo);
        if (m_propertyIndex < typeMetaInfo->m_properties.size())
        {
            return;
        }

        ++m_typeIndex;
        m_propertyIndex = 0;
        if (m_typeIndex > m_typeMetaInfo->m_parentTypeMetaInfos.size())
        {
            m_typeMetaInfo = nullptr;
            m_typeIndex = 0;

            return;
        }
    }
}

size_t ego::rtti::TypeMetaInfo::PropertyIterator::getValueOffset() const
{
    EGO_ASSERT(m_typeMetaInfo);

    const size_t baseOffset = m_typeIndex > 0 ? m_typeMetaInfo->m_parentTypeMetaInfos[m_typeIndex - 1].m_offset : 0;

    return baseOffset + operator*().m_offset;
}

ego::rtti::TypeMetaInfo::PropertyRange::PropertyRange(const TypeMetaInfo* _typeMetaInfo)
    : m_typeMetaInfo(_typeMetaInfo)
{
}

ego::rtti::TypeMetaInfo::PropertyIterator ego::rtti::TypeMetaInfo::PropertyRange::begin() const
{
    return m_typeMetaInfo ? PropertyIterator(m_typeMetaInfo) : PropertyIterator();
}

ego::rtti::TypeMetaInfo::PropertyIterator ego::rtti::TypeMetaInfo::PropertyRange::end() const
{
    return PropertyIterator();
}

bool ego::rtti::TypeMetaInfo::PropertyRange::empty() const
{
    return begin() == end();
}

ego::rtti::TypeMetaInfo::TypeMetaInfo(TypeMetaInfoID _id, ParentTypeMetaInfoCollection&& _parentTypeMetaInfos, PropertyMetaInfoCollection&& _propertyMetaInfos)
    : m_properties(std::move(_propertyMetaInfos)),
      m_id(_id)
{
    EGO_ASSERT(_id);

    for (const ParentTypeMetaInfoContext& parentTypeMetaInfo : _parentTypeMetaInfos)
    {
        EGO_ASSERT(parentTypeMetaInfo.m_info);

        m_parentTypeMetaInfos.emplace_back(parentTypeMetaInfo);
        for (const ParentTypeMetaInfoContext& ancestorTypeMetaInfo : parentTypeMetaInfo.m_info->m_parentTypeMetaInfos)
        {
            m_parentTypeMetaInfos.emplace_back(ancestorTypeMetaInfo.m_info, parentTypeMetaInfo.m_offset + ancestorTypeMetaInfo.m_offset);
        }
    }

    for (const PropertyMetaInfoPointer& propertyMetaInfo : m_properties)
    {
        EGO_ASSERT(propertyMetaInfo);
    }
}

ego::rtti::TypeMetaInfo::~TypeMetaInfo() = default;

ego::rtti::TypeMetaInfoID ego::rtti::TypeMetaInfo::getID() const
{
    return m_id;
}

ego::rtti::TypeMetaInfo::PropertyRange ego::rtti::TypeMetaInfo::getProperties() const
{
    return PropertyRange(this);
}

bool ego::rtti::TypeMetaInfo::isBasedOn(const TypeMetaInfo& _baseType) const
{
    return isBasedOn(_baseType.m_id);
}

bool ego::rtti::TypeMetaInfo::isBasedOn(TypeMetaInfoID _baseTypeID) const
{
    if (m_id == _baseTypeID)
    {
        return true;
    }

    for (const ParentTypeMetaInfoContext& parentContext : m_parentTypeMetaInfos)
    {
        if (parentContext.m_info->m_id == _baseTypeID)
        {
            return true;
        }
    }

    return false;
}

void* ego::rtti::TypeMetaInfo::castTo(void* _object, const TypeMetaInfo& _baseType) const
{
    if (&_baseType == this)
    {
        return _object;
    }

    for (const ParentTypeMetaInfoContext& parentContext : m_parentTypeMetaInfos)
    {
        if (parentContext.m_info == &_baseType)
        {
            return static_cast<uint8_t*>(_object) + parentContext.m_offset;
        }
    }

    return nullptr;
}
