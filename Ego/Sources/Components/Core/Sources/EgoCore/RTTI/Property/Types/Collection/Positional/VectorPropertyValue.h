#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorBoolElementPropertyValue.h"

namespace ego::rtti
{
    template <typename Vector>
    class VectorPropertyValue final : public PositionalCollectionPropertyValue
    {
    public:
        VectorPropertyValue(void* _object, const VectorPropertyMetaInfo<Vector>& _propertyMetaInfo);
        VectorPropertyValue(const void* _object, const VectorPropertyMetaInfo<Vector>& _propertyMetaInfo);

        const VectorPropertyMetaInfo<Vector>& getMetaInfo() const override;
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

        EGO_RTTI_VIRTUAL(VectorPropertyValue, PositionalCollectionPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyValue.hpp"
