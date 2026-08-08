#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class AssociativeCollectionPropertyMetaInfo : public CollectionPropertyMetaInfo
    {
    public:
        AssociativeCollectionPropertyMetaInfo(
            const char* _name,
            size_t _offset,
            bool _isConst,
            const PropertyMetaInfo& _keyMetaInfo,
            const PropertyMetaInfo& _mappedMetaInfo);

        EGO_RTTI_VIRTUAL(AssociativeCollectionPropertyMetaInfo, CollectionPropertyMetaInfo);

        const PropertyMetaInfo& getKeyMetaInfo() const;
        const PropertyMetaInfo& getMappedMetaInfo() const;

    private:
        const PropertyMetaInfo& m_keyMetaInfo;
        const PropertyMetaInfo& m_mappedMetaInfo;
    };
} // namespace ego::rtti
