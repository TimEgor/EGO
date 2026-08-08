#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class PositionalCollectionPropertyMetaInfo : public CollectionPropertyMetaInfo
    {
    public:
        PositionalCollectionPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _elementMetaInfo);

        EGO_RTTI_VIRTUAL(PositionalCollectionPropertyMetaInfo, CollectionPropertyMetaInfo);

        const PropertyMetaInfo& getElementMetaInfo() const;

    private:
        const PropertyMetaInfo& m_elementMetaInfo;
    };
} // namespace ego::rtti
