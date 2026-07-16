#include "GuiFontAtlas.h"

#include <algorithm>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

bool ego::gui::GuiFontAtlas::init(GraphicDevice& _graphicDevice, const GuiFontAtlasDesc& _desc)
{
    release();

    EGO_CHECK_RETURN_FALSE(!_desc.m_fontData.empty());
    EGO_CHECK_RETURN_FALSE(_desc.m_pixelHeight > 0.0f);
    EGO_CHECK_RETURN_FALSE(_desc.m_width > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_height > 0);
    EGO_CHECK_RETURN_FALSE(_desc.m_characterCount > 0);
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
    const int packResult =
        stbtt_PackFontRange(&packContext, _desc.m_fontData.data(), 0, _desc.m_pixelHeight, _desc.m_firstCharacter, static_cast<int>(_desc.m_characterCount), packedGlyphs.data());
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

    m_firstCharacter = _desc.m_firstCharacter;
    m_characterCount = _desc.m_characterCount;
    m_width = _desc.m_width;
    m_height = _desc.m_height;
    m_glyphs.resize(_desc.m_characterCount);

    for (uint32_t index = 0; index < _desc.m_characterCount; ++index)
    {
        const stbtt_packedchar& packedGlyph = packedGlyphs[index];
        GuiFontGlyph& glyph = m_glyphs[index];
        glyph.m_uvRect = GuiRect(
            static_cast<float>(packedGlyph.x0) / static_cast<float>(m_width),
            static_cast<float>(packedGlyph.y0) / static_cast<float>(m_height),
            static_cast<float>(packedGlyph.x1 - packedGlyph.x0) / static_cast<float>(m_width),
            static_cast<float>(packedGlyph.y1 - packedGlyph.y0) / static_cast<float>(m_height));
        glyph.m_size = GuiSize(packedGlyph.xoff2 - packedGlyph.xoff, packedGlyph.yoff2 - packedGlyph.yoff);
        glyph.m_offset = GuiPosition(packedGlyph.xoff, packedGlyph.yoff);
        glyph.m_advance = packedGlyph.xadvance;
    }

    EGO_CHECK_RETURN_CALL_FALSE(initTexture(_graphicDevice, pixels), release());

    m_isInitialized = true;
    return true;
}

void ego::gui::GuiFontAtlas::release()
{
    m_texture = nullptr;
    m_glyphs.clear();
    m_firstCharacter = ' ';
    m_characterCount = 0;
    m_width = 0;
    m_height = 0;
    m_lineHeight = 0.0f;
    m_baseline = 0.0f;
    m_isInitialized = false;
}

const ego::gui::GuiFontGlyph* ego::gui::GuiFontAtlas::getGlyph(char _character) const
{
    if (!m_isInitialized)
    {
        return nullptr;
    }

    const int32_t glyphIndex = static_cast<int32_t>(_character) - static_cast<int32_t>(m_firstCharacter);
    if (glyphIndex < 0 || glyphIndex >= static_cast<int32_t>(m_glyphs.size()))
    {
        return nullptr;
    }

    return &m_glyphs[static_cast<size_t>(glyphIndex)];
}

ego::gui::GuiSize ego::gui::GuiFontAtlas::measureText(std::string_view _text) const
{
    if (!m_isInitialized)
    {
        return GuiSizeZero;
    }

    float maxLineWidth = 0.0f;
    float lineWidth = 0.0f;
    float height = m_lineHeight;

    for (const char character : _text)
    {
        if (character == '\n')
        {
            maxLineWidth = (std::max)(maxLineWidth, lineWidth);
            lineWidth = 0.0f;
            height += m_lineHeight;
            continue;
        }

        const GuiFontGlyph* glyph = getGlyph(character);
        if (glyph)
        {
            lineWidth += glyph->m_advance;
        }
    }

    maxLineWidth = (std::max)(maxLineWidth, lineWidth);
    return GuiSize(maxLineWidth, height);
}

const ego::gpu::Texture2DReference& ego::gui::GuiFontAtlas::getTexture() const
{
    return m_texture;
}

uint32_t ego::gui::GuiFontAtlas::getWidth() const
{
    return m_width;
}

uint32_t ego::gui::GuiFontAtlas::getHeight() const
{
    return m_height;
}

float ego::gui::GuiFontAtlas::getLineHeight() const
{
    return m_lineHeight;
}

float ego::gui::GuiFontAtlas::getBaseline() const
{
    return m_baseline;
}

ego::gui::GuiTextureID ego::gui::GuiFontAtlas::getTextureId() const
{
    return m_isInitialized ? GuiDefaultFontTextureID : InvalidGuiTextureID;
}

bool ego::gui::GuiFontAtlas::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::GuiFontAtlas::initTexture(GraphicDevice& _graphicDevice, const std::vector<uint8_t>& _pixels)
{
    EGO_CHECK_RETURN_FALSE(!_pixels.empty());
    EGO_CHECK_RETURN_FALSE(m_width > 0);
    EGO_CHECK_RETURN_FALSE(m_height > 0);

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GraphicResourceUsageTransferDst | gpu::GraphicResourceUsageShaderResource);
    textureDesc.m_size = gpu::Texture2DSize(m_width, m_height);
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_format = gpu::GraphicResourceFormat::R8UNorm;

    const gpu::InitialGraphicResourceData initialData(_pixels.data(), static_cast<uint32_t>(_pixels.size()), m_width, m_width * m_height);

    gpu::GpuOperationOptions uploadOptions;
    uploadOptions.m_completionMode = gpu::GpuCompletionMode::WaitForCompletion;

    const gpu::GpuTexture2DTicket textureTicket = _graphicDevice.createAndUploadTexture2D(textureDesc, initialData, uploadOptions);
    EGO_CHECK_RETURN_FALSE(textureTicket.m_resource);
    textureTicket.waitReady();

    m_texture = textureTicket.m_resource;
    return m_texture != nullptr;
}
