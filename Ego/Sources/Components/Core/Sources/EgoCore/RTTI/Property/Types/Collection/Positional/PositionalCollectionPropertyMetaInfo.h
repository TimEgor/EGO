#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class PositionalCollectionPropertyMetaInfo : public CollectionPropertyMetaInfo
    {
    public:
        PositionalCollectionPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _elementMetaInfo);

        virtual void* getElementAddress(void* _value, size_t _index) const = 0;
        virtual bool canAddElement() const = 0;
        virtual bool canRemoveElement() const = 0;
        virtual bool addElement(void* _value) const = 0;
        virtual bool removeElement(void* _value, size_t _index) const = 0;

        EGO_RTTI_VIRTUAL(PositionalCollectionPropertyMetaInfo, CollectionPropertyMetaInfo);

        const PropertyMetaInfo* const m_elementMetaInfo;
    };
} // namespace ego::rtti
