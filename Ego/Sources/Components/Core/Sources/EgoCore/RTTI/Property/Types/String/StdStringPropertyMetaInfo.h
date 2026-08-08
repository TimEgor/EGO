#pragma once

#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

namespace ego::rtti
{
    class StdStringPropertyMetaInfo final : public StringPropertyMetaInfo
    {
    public:
        StdStringPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        EGO_RTTI_VIRTUAL(StdStringPropertyMetaInfo, StringPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti
