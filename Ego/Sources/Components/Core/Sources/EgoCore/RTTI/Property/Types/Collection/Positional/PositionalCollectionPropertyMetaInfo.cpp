#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

#include "EgoCore/Assert/Assert.h"

ego::rtti::PositionalCollectionPropertyMetaInfo::PositionalCollectionPropertyMetaInfo(
    const char* _name,
    size_t _offset,
    const PropertyMetaInfo* _elementMetaInfo)
    : CollectionPropertyMetaInfo(_name, _offset),
      m_elementMetaInfo(_elementMetaInfo)
{
    EGO_ASSERT(_elementMetaInfo);
}
