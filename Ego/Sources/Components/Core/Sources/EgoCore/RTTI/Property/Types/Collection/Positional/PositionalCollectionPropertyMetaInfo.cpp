#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

ego::rtti::PositionalCollectionPropertyMetaInfo::PositionalCollectionPropertyMetaInfo(
    const char* _name,
    size_t _offset,
    bool _isConst,
    const PropertyMetaInfo& _elementMetaInfo)
    : CollectionPropertyMetaInfo(_name, _offset, _isConst),
      m_elementMetaInfo(_elementMetaInfo)
{
}

const ego::rtti::PropertyMetaInfo& ego::rtti::PositionalCollectionPropertyMetaInfo::getElementMetaInfo() const
{
    return m_elementMetaInfo;
}
