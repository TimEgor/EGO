#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

namespace ego
{
    namespace hash_combine_details
    {
        template <typename TValue>
        concept BasicHashValue = std::is_arithmetic_v<TValue> || std::is_enum_v<TValue>;

        template <BasicHashValue TValue>
        inline uint64_t HashValue(TValue _value)
        {
            return static_cast<uint64_t>(std::hash<TValue>()(_value));
        }
    } // namespace hash_combine_details

    inline uint16_t HashCombine(uint16_t _seed, uint64_t _val)
    {
        return static_cast<uint16_t>(_seed ^ static_cast<uint16_t>(_val) + 0x9e37U + (_seed << 3) + (_seed >> 1));
    }

    template <hash_combine_details::BasicHashValue TValue>
    inline uint16_t HashCombine(uint16_t _seed, TValue _val)
    {
        return HashCombine(_seed, hash_combine_details::HashValue(_val));
    }

    inline uint32_t HashCombine(std::uint32_t _seed, uint64_t _val)
    {
        return _seed ^ static_cast<uint32_t>(_val) + 0x9e3779b9U + (_seed << 6) + (_seed >> 2);
    }

    template <hash_combine_details::BasicHashValue TValue>
    inline uint32_t HashCombine(uint32_t _seed, TValue _val)
    {
        return HashCombine(_seed, hash_combine_details::HashValue(_val));
    }

    inline uint64_t HashCombine(std::uint64_t _seed, uint64_t _val)
    {
        return _seed ^ _val + 0x9e3779b97f4a7c15LLU + (_seed << 12) + (_seed >> 4);
    }

    template <hash_combine_details::BasicHashValue TValue>
    inline uint64_t HashCombine(uint64_t _seed, TValue _val)
    {
        return HashCombine(_seed, hash_combine_details::HashValue(_val));
    }

    template <typename... TValues>
    inline uint64_t HashValues(uint64_t _hash, const TValues&... _values)
    {
        ((_hash = HashCombine(_hash, _values)), ...);
        return _hash;
    }
} // namespace ego
