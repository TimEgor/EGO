#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoCore/Platform/Window/PlatformWindow.h"

#include "EgoRuntime/Event/EventController.h"

#include "EgoGraphicHardware/PresentationSurface.h"

namespace ego
{
    using WindowSize = PresentationSurfaceSize;
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

    struct WindowDesc final
    {
        const char* m_title = "";
        WindowSize m_size = DefaultWindowSize;
        bool m_showOnInit = true;
    };

    class Window final : public PresentationSurface
    {
    public:
        Window() = default;
        ~Window() override;

        bool init(const PlatformWindowPointer& _platformWindow);
        void release();

        bool isValid() const;

        void show();
        void hide();
        bool isShown() const;

        void* getNativeHandle() const override;
        bool isStable() const;

        const WindowSize& getWindowSize() const;
        const WindowSize& getClientAreaSize() const override;
        const WindowArea& getCutoutsArea() const;
        PlatformWindowPointer getPlatformWindowPointer() const;

        InstancedEventID getSizeEventID() const;

        EGO_RTTI_VIRTUAL(Window, PresentationSurface);

    private:
        bool initWindowInstancedEvents();
        void releaseWindowInstancedEvents();
        void updatePlatformState() const;

        PlatformWindowPointer m_platformWindow = nullptr;
        mutable WindowSize m_windowSize = DefaultWindowSize;
        mutable WindowSize m_clientAreaSize = DefaultWindowSize;
        mutable WindowArea m_cutoutsArea = DefaultWindowArea;
        InstancedEventID m_sizeEventID = InvalidInstancedEventID;
    };

    EGO_POINTER(Window)
} // namespace ego
