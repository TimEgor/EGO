#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Array>
    class ArrayPropertyValue;

    template <typename Array>
    class ArrayPropertyMetaInfo final : public PositionalCollectionPropertyMetaInfo
    {
    public:
        ArrayPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _elementMetaInfo);

        EGO_RTTI_VIRTUAL(ArrayPropertyMetaInfo, PositionalCollectionPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyMetaInfo.hpp"
