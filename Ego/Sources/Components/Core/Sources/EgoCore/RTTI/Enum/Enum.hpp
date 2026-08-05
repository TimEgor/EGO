#include <magic_enum/magic_enum.hpp>

namespace ego::rtti
{
    template <typename Enum>
        requires std::is_enum_v<Enum>
    constexpr std::string_view EnumToString(Enum _value) noexcept
    {
        return magic_enum::enum_name(_value);
    }

    template <typename Enum>
        requires std::is_enum_v<Enum>
    constexpr std::optional<Enum> EnumFromString(std::string_view _name) noexcept
    {
        return magic_enum::enum_cast<Enum>(_name);
    }
} // namespace ego::rtti
