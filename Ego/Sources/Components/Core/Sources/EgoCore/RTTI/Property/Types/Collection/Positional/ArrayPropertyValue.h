#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyValue.h"

namespace ego::rtti
{
    template <typename Array>
    class ArrayPropertyValue final : public PositionalCollectionPropertyValue
    {
    public:
        ArrayPropertyValue(void* _object, const ArrayPropertyMetaInfo<Array>& _propertyMetaInfo);
        ArrayPropertyValue(const void* _object, const ArrayPropertyMetaInfo<Array>& _propertyMetaInfo);

        const ArrayPropertyMetaInfo<Array>& getMetaInfo() const override;
        size_t getSize() const override;
        PropertyValuePointer getElement(size_t _index) override;
        bool setElement(size_t _index, const PropertyValue& _element) override;
        bool canResize() const override;
        bool resize(size_t _size) override;
        bool canAddElement() const override;
        bool addElement() override;
        bool canRemoveElement() const override;
        bool removeElement(size_t _index) override;
        bool canMoveElement() const override;
        bool moveElement(size_t _sourceIndex, size_t _destinationIndex) override;

        EGO_RTTI_VIRTUAL(ArrayPropertyValue, PositionalCollectionPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyValue.hpp"
