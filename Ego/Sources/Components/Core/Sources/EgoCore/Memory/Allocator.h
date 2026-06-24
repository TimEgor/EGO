#pragma once

#include <cstddef>

namespace ego::memory
{
    void* Alloc(std::size_t _size) noexcept;
    void Free(void* _ptr) noexcept;

    void* AllocAligned(std::size_t _size, std::size_t _alignment) noexcept;
    void FreeAligned(void* _ptr) noexcept;
} // namespace ego::memory