#include "GuiCheckBox.h"

#include <algorithm>
#include <utility>

#include "EgoGui/Rendering/GuiFontAtlas.h"

ego::gui::GuiCheckBoxPointer ego::gui::GuiCheckBox::Create()
{
    return new GuiCheckBox();
}

void ego::gui::GuiCheckBox::setText(std::string _text)
{
    m_text = std::move(_text);
}

const std::string& ego::gui::GuiCheckBox::getText() const
{
    return m_text;
}

void ego::gui::GuiCheckBox::setChecked(bool _isChecked)
{
    m_isChecked = _isChecked;
}

bool ego::gui::GuiCheckBox::isChecked() const
{
    return m_isChecked;
}

ego::gui::GuiEventResult ego::gui::GuiCheckBox::onEvent(const GuiInputEvent& _event)
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
        if (containsMouse)
        {
            m_isChecked = !m_isChecked;
        }
        return GuiEventResult::Handled;
    }

    return GuiEventResult::Unhandled;
}

ego::gui::GuiSize ego::gui::GuiCheckBox::onMeasure(const GuiLayoutContext& _context, const GuiSize&)
{
    constexpr float boxSize = 16.0f;
    constexpr float textOffset = 24.0f;
    const GuiSize textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_text) : GuiSizeZero;
    return GuiSize(textOffset + textSize.m_x, (std::max)(boxSize, textSize.m_y));
}

void ego::gui::GuiCheckBox::onPaint(GuiPaintContext& _context) const
{
    constexpr float boxSize = 14.0f;
    const GuiRect boxRect(getRect().m_position, GuiSize(boxSize, boxSize));
    GuiColor boxColor = GuiColor(0.12f, 0.13f, 0.15f, 1.0f);
    if (m_isHovered)
    {
        boxColor = GuiColor(0.18f, 0.21f, 0.24f, 1.0f);
    }
    if (m_isPressed)
    {
        boxColor = GuiColor(0.08f, 0.10f, 0.12f, 1.0f);
    }

    _context.drawBox(boxRect, boxColor);

    if (m_isChecked)
    {
        const GuiRect markRect(boxRect.m_position.m_x + 3.0f, boxRect.m_position.m_y + 3.0f, boxSize - 6.0f, boxSize - 6.0f);
        _context.drawBox(markRect, GuiColor(0.20f, 0.55f, 0.95f, 1.0f));
    }

    const GuiRect textRect(getRect().m_position.m_x + 24.0f, getRect().m_position.m_y, getRect().m_size.m_x - 24.0f, getRect().m_size.m_y);
    _context.drawText(m_text, textRect, GuiColorWhite);
}
