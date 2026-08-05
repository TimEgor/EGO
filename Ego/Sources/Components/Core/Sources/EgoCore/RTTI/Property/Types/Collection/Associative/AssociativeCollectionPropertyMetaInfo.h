#pragma once

#include <functional>

#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyMetaInfo.h"

namespace ego::rtti
{
    class AssociativeCollectionPropertyMetaInfo : public CollectionPropertyMetaInfo
    {
    public:
        using ElementVisitor = std::function<bool(const void* _key, void* _mappedValue)>;

        AssociativeCollectionPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _keyMetaInfo, const PropertyMetaInfo* _mappedMetaInfo);

        virtual void visitElements(void* _value, const ElementVisitor& _visitor) const = 0;
        virtual bool canAddElement() const = 0;
        virtual bool addElement(void* _value) const = 0;

        EGO_RTTI_VIRTUAL(AssociativeCollectionPropertyMetaInfo, CollectionPropertyMetaInfo);

        const PropertyMetaInfo* const m_keyMetaInfo;
        const PropertyMetaInfo* const m_mappedMetaInfo;
    };
} // namespace ego::rtti
