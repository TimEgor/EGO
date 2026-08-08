#pragma once

#include <functional>

#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/CollectionPropertyValue.h"

namespace ego::rtti
{
    enum class AssociativeCollectionElementAction
    {
        Keep,
        Remove
    };

    class AssociativeCollectionPropertyValue : public CollectionPropertyValue
    {
    public:
        using ElementVisitor = std::function<AssociativeCollectionElementAction(PropertyValue& _key, PropertyValue& _mappedValue)>;

        AssociativeCollectionPropertyValue(void* _object, const AssociativeCollectionPropertyMetaInfo& _propertyMetaInfo);
        AssociativeCollectionPropertyValue(const void* _object, const AssociativeCollectionPropertyMetaInfo& _propertyMetaInfo);

        const AssociativeCollectionPropertyMetaInfo& getMetaInfo() const override;
        virtual void visitElements(const ElementVisitor& _visitor) = 0;
        virtual bool canAddElement() const = 0;
        virtual bool addElement() = 0;

        EGO_RTTI_VIRTUAL(AssociativeCollectionPropertyValue, CollectionPropertyValue);
    };
} // namespace ego::rtti
