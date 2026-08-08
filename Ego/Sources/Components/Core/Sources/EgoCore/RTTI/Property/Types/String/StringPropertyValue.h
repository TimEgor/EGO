#pragma once

#include <string_view>

#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

namespace ego::rtti
{
    class StringPropertyValue : public PropertyValue
    {
    public:
        StringPropertyValue(void* _object, const StringPropertyMetaInfo& _propertyMetaInfo);
        StringPropertyValue(const void* _object, const StringPropertyMetaInfo& _propertyMetaInfo);

        const StringPropertyMetaInfo& getMetaInfo() const override;
        virtual std::string_view getValue() const = 0;
        virtual bool setValue(std::string_view _string) = 0;

        EGO_RTTI_VIRTUAL(StringPropertyValue, PropertyValue);
    };
} // namespace ego::rtti
