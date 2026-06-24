#pragma once

#include <cstdint>

namespace ego
{
    inline uint16_t HashCombine(uint16_t _seed, uint64_t _val)
    {
        return static_cast<uint16_t>(_seed ^ static_cast<uint16_t>(_val) + 0x9e37U + (_seed << 3) + (_seed >> 1));
    }

    inline uint32_t HashCombine(std::uint32_t _seed, uint64_t _val)
    {
        return _seed ^ static_cast<uint32_t>(_val) + 0x9e3779b9U + (_seed << 6) + (_seed >> 2);
    }

    inline uint64_t HashCombine(std::uint64_t _seed, uint64_t _val)
    {
        return _seed ^ _val + 0x9e3779b97f4a7c15LLU + (_seed << 12) + (_seed >> 4);
    }
} // namespace ego
