#pragma once

#include <vector>

#include "DrawData.h"
#include "Image.h"
#include "EgoGui/Viewport/ViewportTypes.h"

namespace ego::gui
{
    struct ViewportFrame final
    {
        ViewportID m_viewportID = InvalidViewportID;
        DrawData m_drawData;
    };

    struct Frame final
    {
        using ResourceCollection = ImageBindingCollection;
        using ViewportCollection = std::vector<ViewportFrame>;

        ViewportID m_primaryViewportID = InvalidViewportID;
        ResourceCollection m_resources;
        ViewportCollection m_viewports;
    };
} // namespace ego::gui
