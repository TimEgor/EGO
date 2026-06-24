#include "RTTI.h"

#include "EgoCore/Assert/AssertCore.h"

ego::rtti::TypeMetaInfo::TypeMetaInfo(TypeMetaInfoID _id, size_t _size, ParentTypeMetaInfoCollection&& _parentTypeMetaInfos)
    : m_parentTypeMetaInfos(std::move(_parentTypeMetaInfos)),
      m_size(_size),
      m_id(_id)
{
    EGO_ASSERT(_size);
    EGO_ASSERT(_id);

    for (const ParentTypeMetaInfoContext& parentTypeMetaInfo : m_parentTypeMetaInfos)
    {
        EGO_ASSERT(parentTypeMetaInfo.m_info);
        EGO_ASSERT(parentTypeMetaInfo.m_typeOffset < m_size);
    }
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
        const TypeMetaInfo* parentInfo = parentContext.m_info;
        if (parentInfo->isBasedOn(_baseTypeID))
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
        const TypeMetaInfo* parentInfo = parentContext.m_info;
        void* parentObject = static_cast<uint8_t*>(_object) + parentContext.m_typeOffset;

        void* baseCast = parentInfo->castTo(parentObject, _baseType);
        if (baseCast)
        {
            return baseCast;
        }
    }

    return nullptr;
}
