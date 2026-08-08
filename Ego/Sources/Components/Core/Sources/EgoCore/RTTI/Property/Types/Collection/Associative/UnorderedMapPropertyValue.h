#pragma once

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename UnorderedMap>
    class UnorderedMapPropertyValue final : public AssociativeCollectionPropertyValue
    {
    public:
        UnorderedMapPropertyValue(void* _object, const UnorderedMapPropertyMetaInfo<UnorderedMap>& _propertyMetaInfo);
        UnorderedMapPropertyValue(const void* _object, const UnorderedMapPropertyMetaInfo<UnorderedMap>& _propertyMetaInfo);

        const UnorderedMapPropertyMetaInfo<UnorderedMap>& getMetaInfo() const override;
        size_t getSize() const override;
        void visitElements(const ElementVisitor& _visitor) override;
        bool canAddElement() const override;
        bool addElement() override;

        EGO_RTTI_VIRTUAL(UnorderedMapPropertyValue, AssociativeCollectionPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyValue.hpp"
