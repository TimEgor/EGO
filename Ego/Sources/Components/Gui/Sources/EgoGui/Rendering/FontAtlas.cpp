#include "FontAtlas.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Core/TextEncoding.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

bool ego::gui::FontAtlas::init(const FontAtlasDesc& _desc)
{
    release();

    EGO_CHECK_RETURN_FALSE(!_desc.m_fontData.empty());
    EGO_CHECK_RETURN_FALSE(_desc.m_pixelHeight > 0.0f);
    EGO_CHECK_RETURN_FALSE(_desc.m_width > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_height > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_characterCount > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_firstCodepoint <= static_cast<uint32_t>((std::numeric_limits<int>::max)()));
    EGO_CHECK_RETURN_FALSE(_desc.m_characterCount <= static_cast<uint32_t>((std::numeric_limits<int>::max)()));
    const uint64_t lastCodepoint = static_cast<uint64_t>(_desc.m_firstCodepoint) + _desc.m_characterCount - 1;
    EGO_CHECK_RETURN_FALSE(lastCodepoint <= 0x10FFFF);
    EGO_CHECK_RETURN_FALSE(_desc.m_oversampleX > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_oversampleY > 0);

    const int fontOffset = stbtt_GetFontOffsetForIndex(_desc.m_fontData.data(), 0);
    EGO_CHECK_RETURN_FALSE(fontOffset >= 0);

    std::vector<stbtt_packedchar> packedGlyphs;
    packedGlyphs.resize(_desc.m_characterCount);

    std::vector<uint8_t> pixels;
    pixels.assign(static_cast<size_t>(_desc.m_width) * _desc.m_height, 0);

    stbtt_pack_context packContext;
    const int packBeginResult = stbtt_PackBegin(&packContext, pixels.data(), static_cast<int>(_desc.m_width), static_cast<int>(_desc.m_height), 0, 1, nullptr);
    EGO_CHECK_RETURN_CALL_FALSE(packBeginResult != 0, release());

    stbtt_PackSetOversampling(&packContext, static_cast<unsigned int>(_desc.m_oversampleX), static_cast<unsigned int>(_desc.m_oversampleY));
    const int packResult = stbtt_PackFontRange(
        &packContext,
        _desc.m_fontData.data(),
        0,
        _desc.m_pixelHeight,
        static_cast<int>(_desc.m_firstCodepoint),
        static_cast<int>(_desc.m_characterCount),
        packedGlyphs.data());
    stbtt_PackEnd(&packContext);
    EGO_CHECK_RETURN_CALL_FALSE(packResult != 0, release());

    stbtt_fontinfo fontInfo;
    EGO_CHECK_RETURN_CALL_FALSE(stbtt_InitFont(&fontInfo, _desc.m_fontData.data(), fontOffset) != 0, release());

    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    const float scale = stbtt_ScaleForPixelHeight(&fontInfo, _desc.m_pixelHeight);
    m_baseline = static_cast<float>(ascent) * scale;
    m_lineHeight = static_cast<float>(ascent - descent + lineGap) * scale;
    if (m_lineHeight <= 0.0f)
    {
        m_lineHeight = _desc.m_pixelHeight;
    }

    m_firstCodepoint = _desc.m_firstCodepoint;
    m_characterCount = _desc.m_characterCount;
    m_width = _desc.m_width;
    m_height = _desc.m_height;
    m_glyphs.resize(_desc.m_characterCount);

    for (uint32_t index = 0; index < _desc.m_characterCount; ++index)
    {
        const stbtt_packedchar& packedGlyph = packedGlyphs[index];
        FontGlyph& glyph = m_glyphs[index];
        glyph.m_uvRect = Rect(
            static_cast<float>(packedGlyph.x0) / static_cast<float>(m_width),
            static_cast<float>(packedGlyph.y0) / static_cast<float>(m_height),
            static_cast<float>(packedGlyph.x1 - packedGlyph.x0) / static_cast<float>(m_width),
            static_cast<float>(packedGlyph.y1 - packedGlyph.y0) / static_cast<float>(m_height));
        glyph.m_size = Size(packedGlyph.xoff2 - packedGlyph.xoff, packedGlyph.yoff2 - packedGlyph.yoff);
        glyph.m_offset = Position(packedGlyph.xoff, packedGlyph.yoff);
        glyph.m_advance = packedGlyph.xadvance;
    }

    m_image = Image::Create(DefaultFontImageID, m_width, m_height, ImageFormat::R8, std::move(pixels));
    EGO_CHECK_RETURN_CALL_FALSE(m_image, release());

    m_isInitialized = true;
    return true;
}

void ego::gui::FontAtlas::release()
{
    m_image = nullptr;
    m_glyphs.clear();
    m_firstCodepoint = ' ';
    m_characterCount = 0;
    m_width = 0;
    m_height = 0;
    m_lineHeight = 0.0f;
    m_baseline = 0.0f;
    m_isInitialized = false;
}

const ego::gui::FontGlyph* ego::gui::FontAtlas::getGlyph(uint32_t _codepoint) const
{
    if (!m_isInitialized)
    {
        return nullptr;
    }

    if (_codepoint < m_firstCodepoint)
    {
        return nullptr;
    }

    const uint32_t glyphIndex = _codepoint - m_firstCodepoint;
    return glyphIndex < m_glyphs.size() ? &m_glyphs[glyphIndex] : nullptr;
}

ego::gui::Size ego::gui::FontAtlas::measureText(std::string_view _text) const
{
    if (!m_isInitialized)
    {
        return SizeZero;
    }

    float maxLineWidth = 0.0f;
    float lineWidth = 0.0f;
    float height = m_lineHeight;

    size_t byteOffset = 0;
    uint32_t codepoint = 0;
    while (DecodeNextUtf8(_text, byteOffset, codepoint))
    {
        if (codepoint == '\n')
        {
            maxLineWidth = (std::max)(maxLineWidth, lineWidth);
            lineWidth = 0.0f;
            height += m_lineHeight;
            continue;
        }

        const FontGlyph* glyph = getGlyph(codepoint);
        if (glyph)
        {
            lineWidth += glyph->m_advance;
        }
    }

    maxLineWidth = (std::max)(maxLineWidth, lineWidth);
    return Size(maxLineWidth, height);
}

const ego::gui::ImagePointer& ego::gui::FontAtlas::getImage() const
{
    return m_image;
}

uint32_t ego::gui::FontAtlas::getWidth() const
{
    return m_width;
}

uint32_t ego::gui::FontAtlas::getHeight() const
{
    return m_height;
}

float ego::gui::FontAtlas::getLineHeight() const
{
    return m_lineHeight;
}

float ego::gui::FontAtlas::getBaseline() const
{
    return m_baseline;
}

ego::gui::ImageID ego::gui::FontAtlas::getImageID() const
{
    return m_isInitialized && m_image ? m_image->getID() : InvalidImageID;
}

bool ego::gui::FontAtlas::isInitialized() const
{
    return m_isInitialized && m_image;
}
