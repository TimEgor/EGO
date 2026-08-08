#pragma once

#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

namespace ego::rtti
{
    class FileNamePropertyMetaInfo final : public StringPropertyMetaInfo
    {
    public:
        FileNamePropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        EGO_RTTI_VIRTUAL(FileNamePropertyMetaInfo, StringPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti
