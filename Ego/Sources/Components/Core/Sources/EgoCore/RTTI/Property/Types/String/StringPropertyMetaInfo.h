#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    class StringPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        StringPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        EGO_RTTI_VIRTUAL(StringPropertyMetaInfo, PropertyMetaInfo);
    };
} // namespace ego::rtti
