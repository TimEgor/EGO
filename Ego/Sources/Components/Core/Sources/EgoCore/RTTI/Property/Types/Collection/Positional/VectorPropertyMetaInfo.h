#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Vector>
    class VectorPropertyValue;

    template <typename Vector>
    class VectorPropertyMetaInfo final : public PositionalCollectionPropertyMetaInfo
    {
    public:
        VectorPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const PropertyMetaInfo& _elementMetaInfo);

        EGO_RTTI_VIRTUAL(VectorPropertyMetaInfo, PositionalCollectionPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyMetaInfo.hpp"
