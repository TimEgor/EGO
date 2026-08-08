#pragma once

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::gui
{
    class Backend;

    class Layer
    {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual bool draw(Backend& _backend);

    protected:
        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode = TextureSamplingMode::Color) const;

        virtual void drawGui() = 0;

    private:
        Backend* m_activeBackend = nullptr;
    };
} // namespace ego::gui
