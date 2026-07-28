#pragma once

#include <cstdint>

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::gui
{
    using GuiLayerID = uint64_t;
    inline constexpr GuiLayerID InvalidGuiLayerID = 0;

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
