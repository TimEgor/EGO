#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Array>
    class ArrayPropertyMetaInfo final : public PositionalCollectionPropertyMetaInfo
    {
    public:
        ArrayPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _elementMetaInfo);

        size_t getSize(const void* _value) const override;
        void* getElementAddress(void* _value, size_t _index) const override;
        bool canAddElement() const override;
        bool canRemoveElement() const override;
        bool addElement(void* _value) const override;
        bool removeElement(void* _value, size_t _index) const override;

        EGO_RTTI_VIRTUAL(ArrayPropertyMetaInfo, PositionalCollectionPropertyMetaInfo);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyMetaInfo.hpp"
