#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Pointer>
    class ObjectPropertyMetaInfo final : public PropertyMetaInfo
    {
    public:
        ObjectPropertyMetaInfo(const char* _name, size_t _offset);

        EGO_RTTI_VIRTUAL(ObjectPropertyMetaInfo, PropertyMetaInfo);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Object/ObjectPropertyMetaInfo.hpp"
