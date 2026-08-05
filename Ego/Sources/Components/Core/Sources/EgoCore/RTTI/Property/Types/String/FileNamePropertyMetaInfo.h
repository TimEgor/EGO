#pragma once

#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

namespace ego::rtti
{
    class FileNamePropertyMetaInfo final : public StringPropertyMetaInfo
    {
    public:
        FileNamePropertyMetaInfo(const char* _name, size_t _offset);

        const char* getValue(const void* _value) const override;
        void setValue(void* _value, const char* _string) const override;

        EGO_RTTI_VIRTUAL(FileNamePropertyMetaInfo, StringPropertyMetaInfo);
    };
} // namespace ego::rtti
