#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Struct>
    class StructPropertyMetaInfo final : public PropertyMetaInfo
    {
    public:
        StructPropertyMetaInfo(const char* _name, size_t _offset, const TypeMetaInfo* _valueTypeMetaInfo);

        EGO_RTTI_VIRTUAL(StructPropertyMetaInfo, PropertyMetaInfo);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.hpp"
