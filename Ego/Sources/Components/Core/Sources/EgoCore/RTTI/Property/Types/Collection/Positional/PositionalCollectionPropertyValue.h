#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class PositionalCollectionPropertyValue : public CollectionPropertyValue
    {
    public:
        PositionalCollectionPropertyValue(void* _object, const PositionalCollectionPropertyMetaInfo& _propertyMetaInfo);
        PositionalCollectionPropertyValue(const void* _object, const PositionalCollectionPropertyMetaInfo& _propertyMetaInfo);

        const PositionalCollectionPropertyMetaInfo& getMetaInfo() const override;
        virtual PropertyValuePointer getElement(size_t _index) = 0;
        virtual bool setElement(size_t _index, const PropertyValue& _element) = 0;
        virtual bool canResize() const = 0;
        virtual bool resize(size_t _size) = 0;
        virtual bool canAddElement() const = 0;
        virtual bool addElement() = 0;
        virtual bool canRemoveElement() const = 0;
        virtual bool removeElement(size_t _index) = 0;
        virtual bool canMoveElement() const = 0;
        virtual bool moveElement(size_t _sourceIndex, size_t _destinationIndex) = 0;

        EGO_RTTI_VIRTUAL(PositionalCollectionPropertyValue, CollectionPropertyValue);
    };
} // namespace ego::rtti
