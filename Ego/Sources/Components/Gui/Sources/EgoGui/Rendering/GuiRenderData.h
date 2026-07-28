#pragma once

#include <vector>

#include "EgoGraphicHardware/GraphicObjects/Texture.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "DrawData.h"

namespace ego::gui
{
    struct ViewportRenderData final
    {
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        DrawData m_drawData;
    };

    struct GuiRenderData final
    {
        using TextureViewCollection = std::vector<gpu::TextureViewPointer>;
        using ViewportCollection = std::vector<ViewportRenderData>;

        TextureViewCollection m_resourceTextureViews;
        ViewportCollection m_viewports;
    };
} // namespace ego::gui
