#pragma once

#include <cstdint>
#include <limits>
#include <string>

#include "EgoCore/Math/Vector.h"

namespace ego::gui
{
    using ViewportID = uint32_t;
    inline constexpr ViewportID InvalidViewportID = 0;

    inline constexpr float AutomaticViewportPositionCoordinate = static_cast<float>((std::numeric_limits<int32_t>::min)());
    inline constexpr FloatVector2 AutomaticViewportPosition = FloatVector2(AutomaticViewportPositionCoordinate, AutomaticViewportPositionCoordinate);

    enum class ViewportRole
    {
        Primary,
        Secondary
    };

    struct ViewportCreateRequest final
    {
        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        std::string m_title = "EGO Viewport";
        FloatVector2 m_position = AutomaticViewportPosition;
        FloatVector2 m_size = FloatVector2Zero;
    };
} // namespace ego::gui
