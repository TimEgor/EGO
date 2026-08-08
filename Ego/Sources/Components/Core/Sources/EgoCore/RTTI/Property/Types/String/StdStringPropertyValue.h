#pragma once

#include <string_view>

#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyValue.h"

namespace ego::rtti
{
    class StdStringPropertyValue final : public StringPropertyValue
    {
    public:
        StdStringPropertyValue(void* _object, const StdStringPropertyMetaInfo& _propertyMetaInfo);
        StdStringPropertyValue(const void* _object, const StdStringPropertyMetaInfo& _propertyMetaInfo);

        const StdStringPropertyMetaInfo& getMetaInfo() const override;
        std::string_view getValue() const override;
        bool setValue(std::string_view _string) override;

        EGO_RTTI_VIRTUAL(StdStringPropertyValue, StringPropertyValue);
    };
} // namespace ego::rtti
