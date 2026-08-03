#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiRenderData.h"

namespace ego::gui
{
    class GuiRender : public NonCopyable
    {
    public:
        using TargetCollection = std::vector<gpu::TextureViewPointer>;

        GuiRender() = default;
        ~GuiRender() override = default;

        virtual bool init() = 0;
        virtual void clearResources() = 0;

        virtual bool prepare(GuiRenderData&& _renderData) = 0;
        virtual bool render(const TargetCollection& _targets) = 0;
    };

    EGO_POINTER(GuiRender);
} // namespace ego::gui
