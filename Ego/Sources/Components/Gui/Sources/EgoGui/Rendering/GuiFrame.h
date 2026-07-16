#pragma once

#include <vector>

#include "GuiDrawData.h"
#include "EgoGui/Viewport/GuiViewportTypes.h"

namespace ego::gui
{
    struct GuiViewportFrame final
    {
        GuiViewportID m_viewportID = InvalidGuiViewportID;
        GuiDrawData m_drawData;
    };

    struct GuiFrame final
    {
        using ViewportCollection = std::vector<GuiViewportFrame>;

        GuiViewportID m_primaryViewportID = InvalidGuiViewportID;
        GuiFontAtlasPointer m_fontAtlas = nullptr;
        ViewportCollection m_viewports;
    };
} // namespace ego::gui
