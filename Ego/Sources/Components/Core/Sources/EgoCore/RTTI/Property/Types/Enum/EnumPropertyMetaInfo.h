#pragma once

#include <limits>
#include <span>
#include <string_view>
#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    inline constexpr size_t InvalidEnumValueIndex = std::numeric_limits<size_t>::max();

    class EnumPropertyMetaInfoBase : public PropertyMetaInfo
    {
    public:
        using NameCollection = std::span<const std::string_view>;

        EnumPropertyMetaInfoBase(const char* _name, size_t _offset);

        virtual NameCollection getNames() const = 0;
        virtual size_t getValueIndex(const void* _value) const = 0;
        virtual void setValueIndex(void* _value, size_t _index) const = 0;

        EGO_RTTI_VIRTUAL(EnumPropertyMetaInfoBase, PropertyMetaInfo);
    };

    template <typename Enum>
        requires std::is_enum_v<Enum>
    class EnumPropertyMetaInfo final : public EnumPropertyMetaInfoBase
    {
    public:
        EnumPropertyMetaInfo(const char* _name, size_t _offset);

        NameCollection getNames() const override;
        size_t getValueIndex(const void* _value) const override;
        void setValueIndex(void* _value, size_t _index) const override;

        EGO_RTTI_VIRTUAL(EnumPropertyMetaInfo, EnumPropertyMetaInfoBase);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.hpp"
