#pragma once

#include <cstdint>

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    using WindowSize = UInt16Vector2;
    using WindowPoint = Int32Vector2;

    inline constexpr WindowSize DefaultWindowSize = UInt16Vector2Zero;
    inline constexpr WindowPoint DefaultWindowPoint = Int32Vector2Zero;

    struct WindowArea final
    {
        using ValueType = uint16_t;

        ValueType m_top = 0;
        ValueType m_bottom = 0;
        ValueType m_right = 0;
        ValueType m_left = 0;

        constexpr WindowArea() = default;

        constexpr WindowArea(ValueType _top, ValueType _bottom, ValueType _right, ValueType _left)
            : m_top(_top),
              m_bottom(_bottom),
              m_right(_right),
              m_left(_left)
        {
        }
    };

    inline constexpr WindowArea DefaultWindowArea = WindowArea(0, 0, 0, 0);

    using WindowKeyboardKey = uint32_t;
    using WindowTextCodepoint = uint32_t;

    enum class WindowKeyboardInputAction
    {
        Pressed,
        Released
    };

    struct WindowKeyboardInputData final
    {
        WindowKeyboardKey m_key = 0;
        uint32_t m_scanCode = 0;
        bool m_isRepeat = false;
        bool m_isExtended = false;
        WindowKeyboardInputAction m_action = WindowKeyboardInputAction::Pressed;
    };

    struct WindowTextInputData final
    {
        WindowTextCodepoint m_codepoint = 0;
    };

    struct WindowDesc final
    {
        const char* m_title = "";
        WindowSize m_size = DefaultWindowSize;
        bool m_showOnInit = true;
    };

    class Window
    {
    public:
        Window() = default;
        virtual ~Window() = default;

        virtual bool init(const WindowDesc& _desc) = 0;
        virtual void release() = 0;

        virtual bool isValid() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;

        virtual void* getNativeHandle() const = 0;
        virtual bool isStable() const = 0;
        virtual bool screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const = 0;

        virtual const WindowSize& getWindowSize() const = 0;
        virtual const WindowSize& getClientAreaSize() const = 0;
        virtual const WindowArea& getCutoutsArea() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Window);
    };

    EGO_POINTER(Window);
} // namespace ego
