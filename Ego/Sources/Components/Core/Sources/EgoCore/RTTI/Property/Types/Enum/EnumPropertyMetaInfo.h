#pragma once

#include <span>
#include <string_view>
#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    template <typename Enum>
    requires std::is_enum_v<Enum>
    class TypedEnumPropertyValue;

    class EnumPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        using NameCollection = std::span<const std::string_view>;

        EnumPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        virtual NameCollection getNames() const = 0;

        EGO_RTTI_VIRTUAL(EnumPropertyMetaInfo, PropertyMetaInfo);
    };

    template <typename Enum>
    requires std::is_enum_v<Enum>
    class TypedEnumPropertyMetaInfo final : public EnumPropertyMetaInfo
    {
    public:
        TypedEnumPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst);

        EGO_RTTI_VIRTUAL(TypedEnumPropertyMetaInfo, EnumPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
        NameCollection getNames() const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.hpp"
