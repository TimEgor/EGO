#pragma once

#include <unordered_map>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

struct ImTextureData;

namespace ego::gui
{
    class ImGuiTextureStorage final : public NonCopyable
    {
    public:
        ~ImGuiTextureStorage() override;

        bool synchronize(ImTextureData& _textureData);
        gpu::TextureViewReference getTextureView(ImTextureData& _textureData) const;
        void release();

    private:
        using TextureCollection = std::unordered_map<ImTextureData*, gpu::TextureViewReference>;

        bool createTexture(ImTextureData& _textureData);
        bool replaceTexture(ImTextureData& _textureData);
        void destroyTexture(ImTextureData& _textureData);
        gpu::TextureViewReference createTextureResource(ImTextureData& _textureData) const;

        TextureCollection m_textures;
    };
} // namespace ego::gui
