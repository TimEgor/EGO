#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "EgoCore/Platform/FileSystem/FileSystem.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Core/GuiTypes.h"

namespace ego
{
    class GraphicDevice;
} // namespace ego

namespace ego::gui
{
    inline constexpr GuiTextureID GuiDefaultFontTextureID = 1;

    struct GuiFontGlyph final
    {
        GuiRect m_uvRect;
        GuiSize m_size = GuiSizeZero;
        GuiPosition m_offset = GuiPositionZero;
        float m_advance = 0.0f;
    };

    struct GuiFontAtlasDesc final
    {
        FileContent m_fontData;
        float m_pixelHeight = 16.0f;
        uint32_t m_width = 512;
        uint32_t m_height = 512;
        char m_firstCharacter = ' ';
        uint32_t m_characterCount = 95;
        uint32_t m_oversampleX = 2;
        uint32_t m_oversampleY = 1;
    };

    class GuiFontAtlas final
    {
    public:
        GuiFontAtlas() = default;

        bool init(GraphicDevice& _graphicDevice, const GuiFontAtlasDesc& _desc);
        void release();

        const GuiFontGlyph* getGlyph(char _character) const;
        GuiSize measureText(std::string_view _text) const;

        const gpu::Texture2DReference& getTexture() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        float getLineHeight() const;
        float getBaseline() const;
        GuiTextureID getTextureId() const;
        bool isInitialized() const;

    private:
        bool initTexture(GraphicDevice& _graphicDevice, const std::vector<uint8_t>& _pixels);

        char m_firstCharacter = ' ';
        uint32_t m_characterCount = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        float m_lineHeight = 0.0f;
        float m_baseline = 0.0f;
        gpu::Texture2DReference m_texture = nullptr;
        std::vector<GuiFontGlyph> m_glyphs;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
