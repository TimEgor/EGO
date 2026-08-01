#pragma once

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::gui
{
    class GuiBackend;
    class GuiController;

    class GuiLayer
    {
        friend class GuiController;

    public:
        virtual ~GuiLayer() = default;

    protected:
        GuiLayer() = default;

        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode = TextureSamplingMode::Color) const;

        virtual void drawGui() = 0;

    private:
        bool draw(GuiBackend& _backend);

        GuiBackend* m_activeBackend = nullptr;
    };
} // namespace ego::gui
