#include "GuiPaintContext.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "EgoCore/Assert/Assert.h"

namespace
{
    constexpr float CircleAngle = 6.28318530718f;
    constexpr uint32_t CircleSegmentCount = 24;

    ego::gui::GuiRect IntersectRects(const ego::gui::GuiRect& _left, const ego::gui::GuiRect& _right)
    {
        const float left = (std::max)(_left.getLeft(), _right.getLeft());
        const float top = (std::max)(_left.getTop(), _right.getTop());
        const float right = (std::min)(_left.getRight(), _right.getRight());
        const float bottom = (std::min)(_left.getBottom(), _right.getBottom());

        return ego::gui::GuiRect(left, top, (std::max)(0.0f, right - left), (std::max)(0.0f, bottom - top));
    }
} // namespace

ego::gui::GuiPaintContext::GuiPaintContext(GuiDrawData& _drawData, const GuiRect& _viewportRect, const GuiFontAtlasPointer& _fontAtlas)
    : m_drawData(_drawData),
      m_fontAtlas(_fontAtlas)
{
    m_clipStack.push_back(_viewportRect);
}

void ego::gui::GuiPaintContext::pushClipRect(const GuiRect& _clipRect)
{
    m_clipStack.push_back(IntersectRects(getCurrentClipRect(), _clipRect));
}

void ego::gui::GuiPaintContext::popClipRect()
{
    EGO_ASSERT(m_clipStack.size() > 1);
    if (m_clipStack.size() > 1)
    {
        m_clipStack.pop_back();
    }
}

void ego::gui::GuiPaintContext::drawBox(const GuiRect& _rect, const GuiColor& _color, GuiTextureID _textureId)
{
    appendQuad(_rect, _color, _textureId);
}

void ego::gui::GuiPaintContext::drawCircle(const GuiPosition& _center, float _radius, const GuiColor& _color, GuiTextureID _textureId)
{
    appendCircle(_center, _radius, _color, _textureId);
}

void ego::gui::GuiPaintContext::drawText(std::string_view _text, const GuiRect& _rect, const GuiColor& _color)
{
    if (!m_fontAtlas || !m_fontAtlas->isInitialized() || _rect.m_size.m_x <= 0.0f || _rect.m_size.m_y <= 0.0f)
    {
        return;
    }

    float cursorX = _rect.m_position.m_x;
    float cursorY = _rect.m_position.m_y;
    const float lineHeight = m_fontAtlas->getLineHeight();
    const float baseline = m_fontAtlas->getBaseline();

    pushClipRect(_rect);
    for (const char character : _text)
    {
        if (character == '\n')
        {
            cursorX = _rect.m_position.m_x;
            cursorY += lineHeight;
            continue;
        }

        if (cursorY >= _rect.getBottom())
        {
            break;
        }

        const GuiFontGlyph* glyph = m_fontAtlas->getGlyph(character);
        if (!glyph)
        {
            continue;
        }

        if (glyph->m_size.m_x > 0.0f && glyph->m_size.m_y > 0.0f)
        {
            const GuiRect glyphRect(cursorX + glyph->m_offset.m_x, cursorY + baseline + glyph->m_offset.m_y, glyph->m_size.m_x, glyph->m_size.m_y);
            appendQuad(glyphRect, glyph->m_uvRect, _color, m_fontAtlas->getTextureId());
        }

        cursorX += glyph->m_advance;
        if (cursorX > _rect.getRight())
        {
            break;
        }
    }
    popClipRect();
}

const ego::gui::GuiRect& ego::gui::GuiPaintContext::getCurrentClipRect() const
{
    EGO_ASSERT(!m_clipStack.empty());
    return m_clipStack.back();
}

ego::gui::GuiDrawData& ego::gui::GuiPaintContext::getDrawData()
{
    return m_drawData;
}

void ego::gui::GuiPaintContext::appendQuad(const GuiRect& _rect, const GuiColor& _color, GuiTextureID _textureId)
{
    appendQuad(_rect, GuiRect(0.0f, 0.0f, 1.0f, 1.0f), _color, _textureId);
}

void ego::gui::GuiPaintContext::appendQuad(const GuiRect& _rect, const GuiRect& _uvRect, const GuiColor& _color, GuiTextureID _textureId)
{
    if (_rect.m_size.m_x <= 0.0f || _rect.m_size.m_y <= 0.0f)
    {
        return;
    }

    const uint32_t firstVertex = static_cast<uint32_t>(m_drawData.m_vertices.size());
    const uint32_t firstIndex = static_cast<uint32_t>(m_drawData.m_indices.size());

    m_drawData.m_vertices.push_back(GuiVertex{GuiPosition(_rect.getLeft(), _rect.getTop()), FloatVector2(_uvRect.getLeft(), _uvRect.getTop()), _color});
    m_drawData.m_vertices.push_back(GuiVertex{GuiPosition(_rect.getRight(), _rect.getTop()), FloatVector2(_uvRect.getRight(), _uvRect.getTop()), _color});
    m_drawData.m_vertices.push_back(GuiVertex{GuiPosition(_rect.getRight(), _rect.getBottom()), FloatVector2(_uvRect.getRight(), _uvRect.getBottom()), _color});
    m_drawData.m_vertices.push_back(GuiVertex{GuiPosition(_rect.getLeft(), _rect.getBottom()), FloatVector2(_uvRect.getLeft(), _uvRect.getBottom()), _color});

    m_drawData.m_indices.push_back(firstVertex + 0);
    m_drawData.m_indices.push_back(firstVertex + 1);
    m_drawData.m_indices.push_back(firstVertex + 2);
    m_drawData.m_indices.push_back(firstVertex + 0);
    m_drawData.m_indices.push_back(firstVertex + 2);
    m_drawData.m_indices.push_back(firstVertex + 3);

    GuiDrawCommand command;
    command.m_clipRect = getCurrentClipRect();
    command.m_textureId = _textureId;
    command.m_firstIndex = firstIndex;
    command.m_indexCount = 6;
    command.m_vertexOffset = 0;
    m_drawData.m_commands.push_back(command);
}

void ego::gui::GuiPaintContext::appendCircle(const GuiPosition& _center, float _radius, const GuiColor& _color, GuiTextureID _textureId)
{
    if (_radius <= 0.0f)
    {
        return;
    }

    const uint32_t firstVertex = static_cast<uint32_t>(m_drawData.m_vertices.size());
    const uint32_t firstIndex = static_cast<uint32_t>(m_drawData.m_indices.size());

    m_drawData.m_vertices.push_back(GuiVertex{_center, FloatVector2(0.5f, 0.5f), _color});

    for (uint32_t segmentIndex = 0; segmentIndex < CircleSegmentCount; ++segmentIndex)
    {
        const float angle = CircleAngle * static_cast<float>(segmentIndex) / static_cast<float>(CircleSegmentCount);
        const GuiPosition position(
            _center.m_x + std::cos(angle) * _radius,
            _center.m_y + std::sin(angle) * _radius);
        const FloatVector2 uv(
            0.5f + std::cos(angle) * 0.5f,
            0.5f + std::sin(angle) * 0.5f);

        m_drawData.m_vertices.push_back(GuiVertex{position, uv, _color});
    }

    for (uint32_t segmentIndex = 0; segmentIndex < CircleSegmentCount; ++segmentIndex)
    {
        const uint32_t nextSegmentIndex = (segmentIndex + 1) % CircleSegmentCount;
        m_drawData.m_indices.push_back(firstVertex);
        m_drawData.m_indices.push_back(firstVertex + 1 + segmentIndex);
        m_drawData.m_indices.push_back(firstVertex + 1 + nextSegmentIndex);
    }

    GuiDrawCommand command;
    command.m_clipRect = getCurrentClipRect();
    command.m_textureId = _textureId;
    command.m_firstIndex = firstIndex;
    command.m_indexCount = CircleSegmentCount * 3;
    command.m_vertexOffset = 0;
    m_drawData.m_commands.push_back(command);
}
