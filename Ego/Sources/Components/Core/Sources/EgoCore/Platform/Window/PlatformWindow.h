#pragma once

#include <cstdint>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    struct PlatformWindowSize final
    {
        uint16_t m_x = 0;
        uint16_t m_y = 0;

        constexpr PlatformWindowSize() = default;

        constexpr PlatformWindowSize(uint16_t _x, uint16_t _y)
            : m_x(_x),
              m_y(_y)
        {
        }
    };

    inline constexpr PlatformWindowSize DefaultPlatformWindowSize = PlatformWindowSize(0, 0);

    struct PlatformWindowArea final
    {
        using ValueType = uint16_t;

        ValueType m_top = 0;
        ValueType m_bottom = 0;
        ValueType m_right = 0;
        ValueType m_left = 0;

        constexpr PlatformWindowArea() = default;

        constexpr PlatformWindowArea(ValueType _top, ValueType _bottom, ValueType _right, ValueType _left)
            : m_top(_top),
              m_bottom(_bottom),
              m_right(_right),
              m_left(_left)
        {
        }
    };

    inline constexpr PlatformWindowArea DefaultPlatformWindowArea = PlatformWindowArea(0, 0, 0, 0);

    struct PlatformWindowDesc final
    {
        const char* m_title = "";
        PlatformWindowSize m_size = DefaultPlatformWindowSize;
        bool m_showOnInit = true;
    };

    class PlatformWindow
    {
    public:
        PlatformWindow() = default;
        virtual ~PlatformWindow() = default;

        virtual bool init(const PlatformWindowDesc& _desc) = 0;
        virtual void release() = 0;

        virtual bool isValid() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;

        virtual void* getNativeHandle() const = 0;
        virtual bool isStable() const = 0;

        virtual const PlatformWindowSize& getWindowSize() const = 0;
        virtual const PlatformWindowSize& getClientAreaSize() const = 0;
        virtual const PlatformWindowArea& getCutoutsArea() const = 0;

        EGO_RTTI_VIRTUAL_BASE(PlatformWindow);
    };

    EGO_POINTER(PlatformWindow);
} // namespace ego
