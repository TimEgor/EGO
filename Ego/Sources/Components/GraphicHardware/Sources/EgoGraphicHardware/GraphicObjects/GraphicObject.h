#pragma once

#include <cstdint>

#include "EgoCore/Pointer/IntrusivePointer.h"

namespace ego::gpu
{
    inline constexpr uint32_t InvalidBindlessIndex = -1;

    class GraphicObject : public MTCountable
    {
    public:
        GraphicObject() = default;
        ~GraphicObject() override = default;

        virtual void* getNativeHandle() const = 0;

        template <typename T>
        T* getNativeHandle() const
        {
            return reinterpret_cast<T*>(getNativeHandle());
        }

        virtual void setName(const char* _name) = 0;
    };

    EGO_INTRUSIVE_POINTER(GraphicObject);
} // namespace ego::gpu
