#pragma once

#include <cstdint>

namespace ego::gui
{
    using DockingSpaceID = uint32_t;
    inline constexpr DockingSpaceID InvalidDockingSpaceID = 0;

    enum class DockingPlacement
    {
        Center,
        Left,
        Right,
        Top,
        Bottom
    };

    struct WindowPlacement final
    {
        DockingSpaceID m_spaceID = InvalidDockingSpaceID;
        DockingPlacement m_placement = DockingPlacement::Center;
        float m_splitRatio = 0.5f;
    };
} // namespace ego::gui
