#pragma once

#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyValue.h"

#include <magic_enum/magic_enum.hpp>

namespace ego::rtti
{
    template <typename Enum>
    requires std::is_enum_v<Enum>
    TypedEnumPropertyMetaInfo<Enum>::TypedEnumPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst)
        : EnumPropertyMetaInfo(_name, _offset, _isConst)
    {
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    PropertyValuePointer TypedEnumPropertyMetaInfo<Enum>::makePropertyValue(void* _object) const
    {
        return MakeIntrusive<TypedEnumPropertyValue<Enum>>(_object, *this);
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    PropertyValuePointer TypedEnumPropertyMetaInfo<Enum>::makePropertyValue(const void* _object) const
    {
        return MakeIntrusive<TypedEnumPropertyValue<Enum>>(_object, *this);
    }

    template <typename Enum>
    requires std::is_enum_v<Enum>
    EnumPropertyMetaInfo::NameCollection TypedEnumPropertyMetaInfo<Enum>::getNames() const
    {
        static constexpr auto Names = magic_enum::enum_names<Enum>();

        return Names;
    }
} // namespace ego::rtti
