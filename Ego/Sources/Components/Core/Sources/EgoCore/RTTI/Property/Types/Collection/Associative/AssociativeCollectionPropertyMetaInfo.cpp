#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

#include "EgoCore/Assert/Assert.h"

ego::rtti::AssociativeCollectionPropertyMetaInfo::AssociativeCollectionPropertyMetaInfo(
    const char* _name,
    size_t _offset,
    const PropertyMetaInfo* _keyMetaInfo,
    const PropertyMetaInfo* _mappedMetaInfo)
    : CollectionPropertyMetaInfo(_name, _offset),
      m_keyMetaInfo(_keyMetaInfo),
      m_mappedMetaInfo(_mappedMetaInfo)
{
    EGO_ASSERT(_keyMetaInfo);
    EGO_ASSERT(_mappedMetaInfo);
}
