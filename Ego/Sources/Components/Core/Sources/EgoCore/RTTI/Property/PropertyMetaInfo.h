#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/RTTI/Type/TypeMetaInfoMacros.h"

namespace ego::rtti
{
    class TypeMetaInfo;

    class PropertyMetaInfo : public NonCopyable
    {
    public:
        PropertyMetaInfo(const char* _name, size_t _offset, const TypeMetaInfo* _valueTypeMetaInfo = nullptr);
        virtual ~PropertyMetaInfo();

        EGO_RTTI_VIRTUAL_BASE(PropertyMetaInfo);

        const char* const m_name;
        const size_t m_offset;
        const TypeMetaInfo* const m_valueTypeMetaInfo;
    };
} // namespace ego::rtti
