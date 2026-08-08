#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/RTTI/Type/TypeMetaInfoMacros.h"

namespace ego::rtti
{
    class PropertyMetaInfo : public NonCopyable
    {
    public:
        PropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);
        ~PropertyMetaInfo() override;

        EGO_RTTI_VIRTUAL_BASE(PropertyMetaInfo);

        virtual PropertyValuePointer makePropertyValue(void* _object) const = 0;
        virtual PropertyValuePointer makePropertyValue(const void* _object) const = 0;

        const char* getName() const;
        size_t getOffset() const;
        bool isConst() const;

    private:
        const char* const m_name;
        const size_t m_offset;
        const bool m_isConst;
    };
} // namespace ego::rtti
