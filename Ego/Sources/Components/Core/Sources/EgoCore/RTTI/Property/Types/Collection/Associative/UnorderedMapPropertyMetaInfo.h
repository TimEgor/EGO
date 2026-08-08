#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename UnorderedMap>
    class UnorderedMapPropertyValue;

    template <typename UnorderedMap>
    class UnorderedMapPropertyMetaInfo final : public AssociativeCollectionPropertyMetaInfo
    {
    public:
        UnorderedMapPropertyMetaInfo(
            const char* _name,
            size_t _offset,
            bool _isConst,
            const PropertyMetaInfo& _keyMetaInfo,
            const PropertyMetaInfo& _mappedMetaInfo);

        EGO_RTTI_VIRTUAL(UnorderedMapPropertyMetaInfo, AssociativeCollectionPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyMetaInfo.hpp"
