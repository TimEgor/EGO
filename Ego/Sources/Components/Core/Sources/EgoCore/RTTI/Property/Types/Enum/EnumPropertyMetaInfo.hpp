#pragma once

#include <optional>

#include <magic_enum/magic_enum.hpp>

namespace ego::rtti
{
    template <typename Enum>
        requires std::is_enum_v<Enum>
    EnumPropertyMetaInfo<Enum>::EnumPropertyMetaInfo(const char* _name, size_t _offset)
        : EnumPropertyMetaInfoBase(_name, _offset)
    {
    }

    template <typename Enum>
        requires std::is_enum_v<Enum>
    EnumPropertyMetaInfoBase::NameCollection EnumPropertyMetaInfo<Enum>::getNames() const
    {
        static constexpr auto Names = magic_enum::enum_names<Enum>();

        return Names;
    }

    template <typename Enum>
        requires std::is_enum_v<Enum>
    size_t EnumPropertyMetaInfo<Enum>::getValueIndex(const void* _value) const
    {
        const std::optional<size_t> valueIndex = magic_enum::enum_index(*static_cast<const Enum*>(_value));

        return valueIndex.value_or(InvalidEnumValueIndex);
    }

    template <typename Enum>
        requires std::is_enum_v<Enum>
    void EnumPropertyMetaInfo<Enum>::setValueIndex(void* _value, size_t _index) const
    {
        static constexpr auto Values = magic_enum::enum_values<Enum>();
        if (_index < Values.size())
        {
            *static_cast<Enum*>(_value) = Values[_index];
        }
    }
} // namespace ego::rtti
