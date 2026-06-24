#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoMath/Vector.h"

#include "EgoEngine/Event/EventController.h"

namespace ego
{
    using WindowSize = UInt16Vector2;
    inline constexpr WindowSize DefaultWindowSize = UInt16Vector2Zero;

    struct WindowArea final
    {
        using ValueType = uint16_t;
        using VectorValueType = Vector4Base<ValueType>;

        union
        {
            VectorValueType m_vector;

            struct
            {
                ValueType m_top;
                ValueType m_bottom;
                ValueType m_right;
                ValueType m_left;
            };
        };

        constexpr WindowArea() = default;

        constexpr WindowArea(const VectorValueType& _vector)
            : m_vector(_vector)
        {
        }

        constexpr WindowArea(ValueType _top, ValueType _bottom, ValueType _right, ValueType _left)
            : m_top(_top),
              m_bottom(_bottom),
              m_right(_right),
              m_left(_left)
        {
        }

        constexpr WindowArea(const WindowArea& _area)
            : m_vector(_area.m_vector)
        {
        }

        WindowArea& operator=(const WindowArea& _area)
        {
            m_vector = _area.m_vector;
            return *this;
        }
    };

    constexpr auto DefaultWindowArea = WindowArea(0, 0, 0, 0);

    class Window
    {
    public:
        Window() = default;

        virtual bool init(const char* _title, const WindowSize& _size) = 0;
        virtual void release() = 0;

        virtual bool isValid() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;

        virtual void* getNativeHandle() const = 0;

        virtual bool isStable() const = 0;

        virtual const WindowSize& getWindowSize() const = 0;
        virtual const WindowSize& getClientAreaSize() const = 0;
        virtual const WindowArea& getCutoutsArea() const = 0;

        InstancedEventID getSizeEventID() const;

        EGO_RTTI_VIRTUAL_BASE(Window);

    protected:
        bool initWindowInstancedEvents();
        void releaseWindowInstancedEvents();

    private:
        InstancedEventID m_sizeEventID = InvalidInstancedEventID;
    };

    EGO_POINTER(Window)
} // namespace ego
