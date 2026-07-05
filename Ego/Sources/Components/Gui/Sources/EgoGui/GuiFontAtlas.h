#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "EgoCore/Platform/FileSystem/FileSystem.h"

#include "GuiTypes.h"

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
        GuiTextureID m_textureId = GuiDefaultFontTextureID;
    };

    class GuiFontAtlas final
    {
    public:
        GuiFontAtlas() = default;

        bool init(const GuiFontAtlasDesc& _desc);
        void release();

        const GuiFontGlyph* getGlyph(char _character) const;
        GuiSize measureText(std::string_view _text) const;

        const std::vector<uint8_t>& getPixels() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        float getLineHeight() const;
        float getBaseline() const;
        GuiTextureID getTextureId() const;
        bool isInitialized() const;

    private:
        char m_firstCharacter = ' ';
        uint32_t m_characterCount = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        float m_lineHeight = 0.0f;
        float m_baseline = 0.0f;
        GuiTextureID m_textureId = InvalidGuiTextureID;
        std::vector<uint8_t> m_pixels;
        std::vector<GuiFontGlyph> m_glyphs;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
