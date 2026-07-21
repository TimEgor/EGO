#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "EgoCore/Platform/FileSystem/FileSystem.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class FontAtlas;
    EGO_POINTER(FontAtlas);

    struct FontGlyph final
    {
        Rect m_uvRect;
        Size m_size = SizeZero;
        Position m_offset = PositionZero;
        float m_advance = 0.0f;
    };

    struct FontAtlasDesc final
    {
        FileContent m_fontData;
        float m_pixelHeight = 16.0f;
        uint32_t m_width = 512;
        uint32_t m_height = 512;
        uint32_t m_firstCodepoint = ' ';
        uint32_t m_characterCount = 95;
        uint32_t m_oversampleX = 2;
        uint32_t m_oversampleY = 1;
    };

    class FontAtlas final
    {
    public:
        FontAtlas() = default;

        bool init(const FontAtlasDesc& _desc);
        void release();

        const FontGlyph* getGlyph(uint32_t _codepoint) const;
        Size measureText(std::string_view _text) const;

        const gpu::TextureViewReference& getTextureView() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        float getLineHeight() const;
        float getBaseline() const;
        bool isInitialized() const;

    private:
        uint32_t m_firstCodepoint = ' ';
        uint32_t m_characterCount = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        float m_lineHeight = 0.0f;
        float m_baseline = 0.0f;
        gpu::TextureViewReference m_textureView = nullptr;
        std::vector<FontGlyph> m_glyphs;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
