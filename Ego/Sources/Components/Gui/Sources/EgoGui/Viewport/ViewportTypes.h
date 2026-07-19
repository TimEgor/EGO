#pragma once

#include <cstdint>
#include <vector>

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    using ViewportID = uint32_t;
    inline constexpr ViewportID InvalidViewportID = 0;

    using ViewportIDCollection = std::vector<ViewportID>;

    enum class ViewportRole
    {
        Primary,
        Secondary
    };

    struct ViewportDesc final
    {
        Size m_size = SizeZero;
    };

    struct ViewportCreateRequest final
    {
        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        ViewportDesc m_desc;

        ViewportCreateRequest() = default;

        ViewportCreateRequest(ViewportRole _role, const ViewportDesc& _desc)
            : m_role(_role),
              m_desc(_desc)
        {
        }
    };
} // namespace ego::gui
