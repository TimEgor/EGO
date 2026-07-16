#include "GuiButton.h"

#include <utility>

#include "EgoGui/Rendering/GuiFontAtlas.h"

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

ego::gui::GuiEventResult ego::gui::GuiButton::onEvent(const GuiInputEvent& _event)
{
    if (_event.m_type == GuiInputEventType::FocusLost)
    {
        m_isHovered = false;
        m_isPressed = false;
        return GuiEventResult::Unhandled;
    }

    if (_event.m_type == GuiInputEventType::PointerLeave)
    {
        m_isHovered = false;
        return GuiEventResult::Unhandled;
    }

    const bool containsMouse = getRect().contains(_event.m_position);
    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        m_isHovered = containsMouse;
        return GuiEventResult::Unhandled;
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left && containsMouse)
    {
        m_isHovered = true;
        m_isPressed = true;
        return GuiEventResult::Handled;
    }

    if (_event.m_type == GuiInputEventType::MouseButtonUp && _event.m_mouseButton == GuiMouseButton::Left && m_isPressed)
    {
        m_isPressed = false;
        m_isHovered = containsMouse;
        return containsMouse && m_onClicked ? m_onClicked() : GuiEventResult::Handled;
    }

    return GuiEventResult::Unhandled;
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
