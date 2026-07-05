#include "GuiTextBlock.h"

#include <utility>

#include "EgoGui/GuiFontAtlas.h"

ego::gui::GuiTextBlockPointer ego::gui::GuiTextBlock::Create()
{
    return new GuiTextBlock();
}

void ego::gui::GuiTextBlock::setText(std::string _text)
{
    m_text = std::move(_text);
}

const std::string& ego::gui::GuiTextBlock::getText() const
{
    return m_text;
}

void ego::gui::GuiTextBlock::setColor(const GuiColor& _color)
{
    m_color = _color;
}

const ego::gui::GuiColor& ego::gui::GuiTextBlock::getColor() const
{
    return m_color;
}

ego::gui::GuiSize ego::gui::GuiTextBlock::onMeasure(const GuiLayoutContext& _context, const GuiSize&)
{
    return _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_text) : GuiSizeZero;
}

void ego::gui::GuiTextBlock::onPaint(GuiPaintContext& _context) const
{
    _context.drawText(m_text, getRect(), m_color);
}
