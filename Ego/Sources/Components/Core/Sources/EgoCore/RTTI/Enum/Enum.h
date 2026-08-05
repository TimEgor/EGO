#pragma once

#include <optional>
#include <string_view>
#include <type_traits>

namespace ego::rtti
{
    template <typename Enum>
        requires std::is_enum_v<Enum>
    constexpr std::string_view EnumToString(Enum _value) noexcept;

    template <typename Enum>
        requires std::is_enum_v<Enum>
    constexpr std::optional<Enum> EnumFromString(std::string_view _name) noexcept;
} // namespace ego::rtti

#include "EgoCore/RTTI/Enum/Enum.hpp"
