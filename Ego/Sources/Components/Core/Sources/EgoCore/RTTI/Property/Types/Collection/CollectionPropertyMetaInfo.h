#pragma once

#include <cstddef>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    class CollectionPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        CollectionPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        EGO_RTTI_VIRTUAL(CollectionPropertyMetaInfo, PropertyMetaInfo);
    };
} // namespace ego::rtti
