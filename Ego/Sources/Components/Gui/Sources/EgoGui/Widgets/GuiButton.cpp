#include "GuiButton.h"

#include <utility>

#include "EgoGui/GuiFontAtlas.h"

ego::gui::GuiButtonPointer ego::gui::GuiButton::Create()
{
    return new GuiButton();
}

void ego::gui::GuiButton::setText(std::string _text)
{
    m_text = std::move(_text);
}

const std::string& ego::gui::GuiButton::getText() const
{
    return m_text;
}

void ego::gui::GuiButton::setOnClicked(GuiClickedHandler _handler)
{
    m_onClicked = std::move(_handler);
}

ego::gui::GuiReply ego::gui::GuiButton::handleEvent(const GuiInputEvent& _event)
{
    const bool containsMouse = _event.m_hasPosition && getRect().contains(_event.m_position);
    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        m_isHovered = containsMouse;
        return GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left && containsMouse)
    {
        m_isHovered = true;
        m_isPressed = true;
        return GuiReply::Handled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonUp && _event.m_mouseButton == GuiMouseButton::Left && m_isPressed)
    {
        m_isPressed = false;
        m_isHovered = containsMouse;
        return containsMouse && m_onClicked ? m_onClicked() : GuiReply::Handled();
    }

    return GuiReply::Unhandled();
}

ego::gui::GuiSize ego::gui::GuiButton::onMeasure(const GuiLayoutContext& _context, const GuiSize&)
{
    const GuiSize textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_text) : GuiSizeZero;
    return GuiSize(textSize.m_x + m_padding.getHorizontal(), textSize.m_y + m_padding.getVertical());
}

void ego::gui::GuiButton::onPaint(GuiPaintContext& _context) const
{
    GuiColor background = m_backgroundColor;
    if (m_isHovered)
    {
        background = GuiColor(0.20f, 0.23f, 0.26f, 1.0f);
    }
    if (m_isPressed)
    {
        background = GuiColor(0.10f, 0.12f, 0.14f, 1.0f);
    }

    _context.drawBox(getRect(), background);

    const GuiRect textRect(
        getRect().m_position.m_x + m_padding.m_left,
        getRect().m_position.m_y + m_padding.m_top,
        getRect().m_size.m_x - m_padding.getHorizontal(),
        getRect().m_size.m_y - m_padding.getVertical());
    _context.drawText(m_text, textRect, m_textColor);
}
