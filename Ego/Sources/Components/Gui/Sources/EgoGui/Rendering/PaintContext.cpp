#include "PaintContext.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Core/TextEncoding.h"

namespace
{
    constexpr float CircleAngle = 6.28318530718f;
    constexpr uint32_t CircleSegmentCount = 24;

    ego::gui::Rect IntersectRects(const ego::gui::Rect& _left, const ego::gui::Rect& _right)
    {
        const float left = (std::max)(_left.getLeft(), _right.getLeft());
        const float top = (std::max)(_left.getTop(), _right.getTop());
        const float right = (std::min)(_left.getRight(), _right.getRight());
        const float bottom = (std::min)(_left.getBottom(), _right.getBottom());

        return ego::gui::Rect(left, top, (std::max)(0.0f, right - left), (std::max)(0.0f, bottom - top));
    }
} // namespace

ego::gui::PaintContext::PaintContext(DrawData& _drawData, const Rect& _viewportRect, const FontAtlasPointer& _fontAtlas, const ThemePointer& _theme)
    : m_drawData(_drawData),
      m_fontAtlas(_fontAtlas),
      m_theme(_theme)
{
    EGO_ASSERT(m_theme);
    m_clipStack.push_back(_viewportRect);
}

void ego::gui::PaintContext::pushClipRect(const Rect& _clipRect)
{
    m_clipStack.push_back(IntersectRects(getCurrentClipRect(), _clipRect));
}

void ego::gui::PaintContext::popClipRect()
{
    EGO_ASSERT(m_clipStack.size() > 1);
    if (m_clipStack.size() > 1)
    {
        m_clipStack.pop_back();
    }
}

void ego::gui::PaintContext::drawBox(const Rect& _rect, const NormalizedColorRGBA& _color, uint32_t _textureIndex)
{
    appendQuad(_rect, _color, _textureIndex);
}

void ego::gui::PaintContext::drawBorder(const Rect& _rect, float _thickness, const NormalizedColorRGBA& _color)
{
    const float maximumThickness = (std::max)(0.0f, (std::min)(_rect.m_size.m_x, _rect.m_size.m_y) * 0.5f);
    const float thickness = (std::clamp)(_thickness, 0.0f, maximumThickness);
    if (thickness <= 0.0f)
    {
        return;
    }

    drawBox(Rect(_rect.getLeft(), _rect.getTop(), _rect.m_size.m_x, thickness), _color);
    drawBox(Rect(_rect.getLeft(), _rect.getBottom() - thickness, _rect.m_size.m_x, thickness), _color);
    drawBox(Rect(_rect.getLeft(), _rect.getTop() + thickness, thickness, _rect.m_size.m_y - thickness * 2.0f), _color);
    drawBox(Rect(_rect.getRight() - thickness, _rect.getTop() + thickness, thickness, _rect.m_size.m_y - thickness * 2.0f), _color);
}

void ego::gui::PaintContext::drawCircle(const Position& _center, float _radius, const NormalizedColorRGBA& _color, uint32_t _textureIndex)
{
    appendCircle(_center, _radius, _color, _textureIndex);
}

void ego::gui::PaintContext::drawTriangle(
    const Position& _firstPosition,
    const Position& _secondPosition,
    const Position& _thirdPosition,
    const NormalizedColorRGBA& _color,
    uint32_t _textureIndex)
{
    appendTriangle(_firstPosition, _secondPosition, _thirdPosition, _color, _textureIndex);
}

void ego::gui::PaintContext::drawText(std::string_view _text, const Rect& _rect, const NormalizedColorRGBA& _color)
{
    if (!m_fontAtlas || !m_fontAtlas->isInitialized() || _rect.m_size.m_x <= 0.0f || _rect.m_size.m_y <= 0.0f)
    {
        return;
    }

    float cursorX = _rect.m_position.m_x;
    float cursorY = _rect.m_position.m_y;
    const float lineHeight = m_fontAtlas->getLineHeight();
    const float baseline = m_fontAtlas->getBaseline();
    const uint32_t textureIndex = m_fontAtlas->getTextureView()->getBindlessIndex();

    pushClipRect(_rect);
    size_t byteOffset = 0;
    uint32_t codepoint = 0;
    while (DecodeNextUtf8(_text, byteOffset, codepoint))
    {
        if (codepoint == '\n')
        {
            cursorX = _rect.m_position.m_x;
            cursorY += lineHeight;
            continue;
        }

        if (cursorY >= _rect.getBottom())
        {
            break;
        }

        const FontGlyph* glyph = m_fontAtlas->getGlyph(codepoint);
        if (!glyph)
        {
            continue;
        }

        if (glyph->m_size.m_x > 0.0f && glyph->m_size.m_y > 0.0f)
        {
            const Rect glyphRect(cursorX + glyph->m_offset.m_x, cursorY + baseline + glyph->m_offset.m_y, glyph->m_size.m_x, glyph->m_size.m_y);
            appendQuad(glyphRect, glyph->m_uvRect, _color, textureIndex);
        }

        cursorX += glyph->m_advance;
        if (cursorX > _rect.getRight())
        {
            break;
        }
    }
    popClipRect();
}

const ego::gui::Rect& ego::gui::PaintContext::getCurrentClipRect() const
{
    EGO_ASSERT(!m_clipStack.empty());
    return m_clipStack.back();
}

const ego::gui::Theme& ego::gui::PaintContext::getTheme() const
{
    EGO_ASSERT(m_theme);
    return *m_theme;
}

void ego::gui::PaintContext::appendQuad(const Rect& _rect, const NormalizedColorRGBA& _color, uint32_t _textureIndex)
{
    appendQuad(_rect, Rect(0.0f, 0.0f, 1.0f, 1.0f), _color, _textureIndex);
}

void ego::gui::PaintContext::appendQuad(const Rect& _rect, const Rect& _uvRect, const NormalizedColorRGBA& _color, uint32_t _textureIndex)
{
    if (_rect.m_size.m_x <= 0.0f || _rect.m_size.m_y <= 0.0f)
    {
        return;
    }

    const uint32_t firstVertex = static_cast<uint32_t>(m_drawData.m_vertices.size());
    const uint32_t firstIndex = static_cast<uint32_t>(m_drawData.m_indices.size());

    m_drawData.m_vertices.push_back(Vertex{Position(_rect.getLeft(), _rect.getTop()), FloatVector2(_uvRect.getLeft(), _uvRect.getTop()), _color});
    m_drawData.m_vertices.push_back(Vertex{Position(_rect.getRight(), _rect.getTop()), FloatVector2(_uvRect.getRight(), _uvRect.getTop()), _color});
    m_drawData.m_vertices.push_back(Vertex{Position(_rect.getRight(), _rect.getBottom()), FloatVector2(_uvRect.getRight(), _uvRect.getBottom()), _color});
    m_drawData.m_vertices.push_back(Vertex{Position(_rect.getLeft(), _rect.getBottom()), FloatVector2(_uvRect.getLeft(), _uvRect.getBottom()), _color});

    m_drawData.m_indices.push_back(firstVertex + 0);
    m_drawData.m_indices.push_back(firstVertex + 1);
    m_drawData.m_indices.push_back(firstVertex + 2);
    m_drawData.m_indices.push_back(firstVertex + 0);
    m_drawData.m_indices.push_back(firstVertex + 2);
    m_drawData.m_indices.push_back(firstVertex + 3);

    appendDrawCommand(firstIndex, 6, _textureIndex);
}

void ego::gui::PaintContext::appendCircle(const Position& _center, float _radius, const NormalizedColorRGBA& _color, uint32_t _textureIndex)
{
    if (_radius <= 0.0f)
    {
        return;
    }

    const uint32_t firstVertex = static_cast<uint32_t>(m_drawData.m_vertices.size());
    const uint32_t firstIndex = static_cast<uint32_t>(m_drawData.m_indices.size());

    m_drawData.m_vertices.push_back(Vertex{_center, FloatVector2(0.5f, 0.5f), _color});

    for (uint32_t segmentIndex = 0; segmentIndex < CircleSegmentCount; ++segmentIndex)
    {
        const float angle = CircleAngle * static_cast<float>(segmentIndex) / static_cast<float>(CircleSegmentCount);
        const Position position(_center.m_x + std::cos(angle) * _radius, _center.m_y + std::sin(angle) * _radius);
        const FloatVector2 uv(0.5f + std::cos(angle) * 0.5f, 0.5f + std::sin(angle) * 0.5f);

        m_drawData.m_vertices.push_back(Vertex{position, uv, _color});
    }

    for (uint32_t segmentIndex = 0; segmentIndex < CircleSegmentCount; ++segmentIndex)
    {
        const uint32_t nextSegmentIndex = (segmentIndex + 1) % CircleSegmentCount;
        m_drawData.m_indices.push_back(firstVertex);
        m_drawData.m_indices.push_back(firstVertex + 1 + segmentIndex);
        m_drawData.m_indices.push_back(firstVertex + 1 + nextSegmentIndex);
    }

    appendDrawCommand(firstIndex, CircleSegmentCount * 3, _textureIndex);
}

void ego::gui::PaintContext::appendTriangle(
    const Position& _firstPosition,
    const Position& _secondPosition,
    const Position& _thirdPosition,
    const NormalizedColorRGBA& _color,
    uint32_t _textureIndex)
{
    const uint32_t firstVertex = static_cast<uint32_t>(m_drawData.m_vertices.size());
    const uint32_t firstIndex = static_cast<uint32_t>(m_drawData.m_indices.size());

    m_drawData.m_vertices.push_back(Vertex{_firstPosition, FloatVector2(0.0f, 0.0f), _color});
    m_drawData.m_vertices.push_back(Vertex{_secondPosition, FloatVector2(1.0f, 0.0f), _color});
    m_drawData.m_vertices.push_back(Vertex{_thirdPosition, FloatVector2(1.0f, 1.0f), _color});

    m_drawData.m_indices.push_back(firstVertex + 0);
    m_drawData.m_indices.push_back(firstVertex + 1);
    m_drawData.m_indices.push_back(firstVertex + 2);

    appendDrawCommand(firstIndex, 3, _textureIndex);
}

void ego::gui::PaintContext::appendDrawCommand(uint32_t _firstIndex, uint32_t _indexCount, uint32_t _textureIndex)
{
    const Rect& clipRect = getCurrentClipRect();
    if (!m_drawData.m_commands.empty())
    {
        DrawCommand& previousCommand = m_drawData.m_commands.back();
        if (previousCommand.m_textureIndex == _textureIndex && previousCommand.m_clipRect == clipRect &&
            previousCommand.m_firstIndex + previousCommand.m_indexCount == _firstIndex)
        {
            previousCommand.m_indexCount += _indexCount;
            return;
        }
    }

    DrawCommand command;
    command.m_clipRect = clipRect;
    command.m_textureIndex = _textureIndex;
    command.m_firstIndex = _firstIndex;
    command.m_indexCount = _indexCount;
    command.m_vertexOffset = 0;
    m_drawData.m_commands.push_back(command);
}
