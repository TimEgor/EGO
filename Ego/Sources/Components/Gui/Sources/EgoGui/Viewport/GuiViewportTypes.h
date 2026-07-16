#pragma once

#include <cstdint>
#include <vector>

#include "EgoGui/Core/GuiTypes.h"

namespace ego::gui
{
    using GuiViewportID = uint32_t;
    inline constexpr GuiViewportID InvalidGuiViewportID = 0;

    using GuiViewportIDCollection = std::vector<GuiViewportID>;

    enum class GuiViewportRole
    {
        Primary,
        Secondary
    };

    struct GuiViewportDesc final
    {
        GuiSize m_size = GuiSizeZero;
    };

    struct GuiViewportCreateRequest final
    {
        GuiViewportID m_id = InvalidGuiViewportID;
        GuiViewportRole m_role = GuiViewportRole::Secondary;
        GuiViewportDesc m_desc;

        GuiViewportCreateRequest() = default;

        GuiViewportCreateRequest(GuiViewportRole _role, const GuiViewportDesc& _desc)
            : m_role(_role),
              m_desc(_desc)
        {
        }
    };
} // namespace ego::gui
