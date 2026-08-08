#pragma once

#include <string_view>

#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyValue.h"

namespace ego::rtti
{
    class FileNamePropertyValue final : public StringPropertyValue
    {
    public:
        FileNamePropertyValue(void* _object, const FileNamePropertyMetaInfo& _propertyMetaInfo);
        FileNamePropertyValue(const void* _object, const FileNamePropertyMetaInfo& _propertyMetaInfo);

        const FileNamePropertyMetaInfo& getMetaInfo() const override;
        std::string_view getValue() const override;
        bool setValue(std::string_view _string) override;

        EGO_RTTI_VIRTUAL(FileNamePropertyValue, StringPropertyValue);
    };
} // namespace ego::rtti
