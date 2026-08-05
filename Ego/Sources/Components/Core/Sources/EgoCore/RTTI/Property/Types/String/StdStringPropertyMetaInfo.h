#pragma once

#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

namespace ego::rtti
{
    class StdStringPropertyMetaInfo final : public StringPropertyMetaInfo
    {
    public:
        StdStringPropertyMetaInfo(const char* _name, size_t _offset);

        const char* getValue(const void* _value) const override;
        void setValue(void* _value, const char* _string) const override;

        EGO_RTTI_VIRTUAL(StdStringPropertyMetaInfo, StringPropertyMetaInfo);
    };
} // namespace ego::rtti
