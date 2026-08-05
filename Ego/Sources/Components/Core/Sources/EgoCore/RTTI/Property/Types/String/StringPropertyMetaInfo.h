#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    class StringPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        StringPropertyMetaInfo(const char* _name, size_t _offset);

        virtual const char* getValue(const void* _value) const = 0;
        virtual void setValue(void* _value, const char* _string) const = 0;

        EGO_RTTI_VIRTUAL(StringPropertyMetaInfo, PropertyMetaInfo);
    };
} // namespace ego::rtti
