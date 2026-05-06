#include "Allocator.h"

#include <cstdlib>
#include <new>

void* ego::memory::Alloc(std::size_t _size) noexcept
{
    void* ptr = std::malloc(_size);
    return ptr;
}

void ego::memory::Free(void* _ptr) noexcept
{
    std::free(_ptr);
}

void* ego::memory::AllocAligned(std::size_t _size, std::size_t _alignment) noexcept
{
#if defined(_MSC_VER)
    void* ptr = _aligned_malloc(_alignment, _alignment);
#else
    const std::size_t alignedSize = (_size + _alignment - 1) & ~(_alignment - 1);
    void* ptr = std::aligned_alloc(_alignment, alignedSize);
#endif

    return ptr;
}

void ego::memory::FreeAligned(void* _ptr) noexcept
{
#if defined(_MSC_VER)
    _aligned_free(_ptr);
#else
    std::free(_ptr);
#endif
}

void* operator new(std::size_t _size)
{
    return ego::memory::Alloc(_size);
}

void operator delete(void* _ptr) noexcept
{
    ego::memory::Free(_ptr);
}

void* operator new[](std::size_t _size)
{
    return ego::memory::Alloc(_size);
}

void operator delete[](void* _ptr) noexcept
{
    ego::memory::Free(_ptr);
}

// sized delete (C++14+)
void operator delete(void* _ptr, std::size_t) noexcept
{
    ego::memory::Free(_ptr);
}

void operator delete[](void* _ptr, std::size_t) noexcept
{
    ego::memory::Free(_ptr);
}

// nothrow
void* operator new(std::size_t _size, const std::nothrow_t&) noexcept
{
    return ego::memory::Alloc(_size);
}

void* operator new[](std::size_t _size, const std::nothrow_t&) noexcept
{
    return ego::memory::Alloc(_size);
}

void operator delete(void* _ptr, const std::nothrow_t&) noexcept
{
    ego::memory::Free(_ptr);
}

void operator delete[](void* _ptr, const std::nothrow_t&) noexcept
{
    ego::memory::Free(_ptr);
}

// aligned new/delete (C++17)
void* operator new(std::size_t _size, std::align_val_t _align)
{
    return ego::memory::AllocAligned(_size, static_cast<std::size_t>(_align));
}

void* operator new[](std::size_t _size, std::align_val_t _align)
{
    return ego::memory::AllocAligned(_size, static_cast<std::size_t>(_align));
}

void operator delete(void* _ptr, std::align_val_t) noexcept
{
    ego::memory::FreeAligned(_ptr);
}

void operator delete[](void* _ptr, std::align_val_t) noexcept
{
    ego::memory::FreeAligned(_ptr);
}

void operator delete(void* _ptr, std::size_t, std::align_val_t) noexcept
{
    ego::memory::FreeAligned(_ptr);
}

void operator delete[](void* _ptr, std::size_t, std::align_val_t) noexcept
{
    ego::memory::FreeAligned(_ptr);
}
