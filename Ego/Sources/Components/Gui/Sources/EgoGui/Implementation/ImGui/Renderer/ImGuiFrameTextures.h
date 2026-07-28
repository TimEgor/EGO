#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiTexture.h"

namespace ego::gui
{
    class ImGuiFrameTextures final : public NonCopyable
    {
    public:
        struct Texture final
        {
            gpu::TextureViewPointer m_textureView = nullptr;
            TextureSamplingMode m_samplingMode = TextureSamplingMode::Color;
        };

        void reset();

        GuiFrameTextureID bind(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode);
        bool resolve(GuiFrameTextureID _textureID, Texture& _texture) const;

    private:
        static bool IsTextureViewValid(const gpu::TextureViewPointer& _textureView);

        std::vector<Texture> m_textures;
    };
} // namespace ego::gui
