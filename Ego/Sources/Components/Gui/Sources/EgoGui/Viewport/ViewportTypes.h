#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    using ViewportID = uint32_t;
    inline constexpr ViewportID InvalidViewportID = 0;

    using ViewportIDCollection = std::vector<ViewportID>;

    inline constexpr float AutomaticViewportPositionCoordinate = static_cast<float>((std::numeric_limits<int32_t>::min)());
    inline constexpr Position AutomaticViewportPosition = Position(AutomaticViewportPositionCoordinate, AutomaticViewportPositionCoordinate);

    enum class ViewportRole
    {
        Primary,
        Secondary
    };

    struct ViewportDesc final
    {
        std::string m_title = "EGO Viewport";
        Position m_position = AutomaticViewportPosition;
        Size m_size = SizeZero;
    };

    struct ViewportCreateRequest final
    {
        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        std::string m_title = "EGO Viewport";
        Position m_position = AutomaticViewportPosition;
        Size m_size = SizeZero;

        ViewportCreateRequest() = default;

        ViewportCreateRequest(ViewportRole _role, const ViewportDesc& _desc)
            : m_role(_role),
              m_title(_desc.m_title),
              m_position(_desc.m_position),
              m_size(_desc.m_size)
        {
        }
    };
} // namespace ego::gui
