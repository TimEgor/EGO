#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

ego::rtti::AssociativeCollectionPropertyMetaInfo::AssociativeCollectionPropertyMetaInfo(
    const char* _name,
    size_t _offset,
    bool _isConst,
    const PropertyMetaInfo& _keyMetaInfo,
    const PropertyMetaInfo& _mappedMetaInfo)
    : CollectionPropertyMetaInfo(_name, _offset, _isConst),
      m_keyMetaInfo(_keyMetaInfo),
      m_mappedMetaInfo(_mappedMetaInfo)
{
}

const ego::rtti::PropertyMetaInfo& ego::rtti::AssociativeCollectionPropertyMetaInfo::getKeyMetaInfo() const
{
    return m_keyMetaInfo;
}

const ego::rtti::PropertyMetaInfo& ego::rtti::AssociativeCollectionPropertyMetaInfo::getMappedMetaInfo() const
{
    return m_mappedMetaInfo;
}
