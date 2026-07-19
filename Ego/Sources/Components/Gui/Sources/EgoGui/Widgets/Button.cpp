#include "Button.h"

#include <utility>

#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::ButtonPointer ego::gui::Button::Create()
{
    return new Button();
}

ego::gui::ButtonPointer ego::gui::Button::Create(std::string _text, ClickedHandler _onClicked)
{
    const ButtonPointer button = new Button();
    button->setText(std::move(_text));
    button->onClick(std::move(_onClicked));
    return button;
}

void ego::gui::Button::setText(std::string _text)
{
    if (m_text == _text)
    {
        return;
    }

    m_text = std::move(_text);
    invalidateLayout();
}

const std::string& ego::gui::Button::getText() const
{
    return m_text;
}

void ego::gui::Button::onClick(ClickedHandler _handler)
{
    m_onClicked.set(std::move(_handler));
}

ego::gui::InputReply ego::gui::Button::onPointerMove(const PointerMoveEvent& _event)
{
    m_isHovered = getLayoutBounds().contains(_event.m_position);
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::Button::onMouseButton(const MouseButtonEvent& _event)
{
    const bool containsMouse = getLayoutBounds().contains(_event.m_position);
    if (_event.m_action == InputButtonAction::Pressed && _event.m_key == MouseInputKey::ButtonLeft && containsMouse && !m_isPressed)
    {
        m_isHovered = true;
        m_isPressed = true;
        return InputReply::FocusAndCapture;
    }

    if (_event.m_action == InputButtonAction::Released && _event.m_key == MouseInputKey::ButtonLeft && m_isPressed)
    {
        m_isPressed = false;
        m_isHovered = containsMouse;
        if (containsMouse)
        {
            m_onClicked.invoke();
        }

        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::Button::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_isHovered = getLayoutBounds().contains(_position);
}

void ego::gui::Button::onPointerLeave(const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::Button::onPointerCaptureLost(const Position&)
{
    m_isHovered = false;
    m_isPressed = false;
}

ego::gui::Size ego::gui::Button::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints&)
{
    const Margin& padding = _context.getTheme().m_button.m_padding;
    const Size textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_text) : SizeZero;
    return Size(textSize.m_x + padding.getHorizontal(), textSize.m_y + padding.getVertical());
}

void ego::gui::Button::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const Margin& padding = theme.m_button.m_padding;
    const NormalizedColorRGBA& background = theme.m_button.m_background.resolve(m_isHovered, m_isPressed);

    const Rect& layoutBounds = getLayoutBounds();
    _context.drawBox(layoutBounds, background);

    const Rect textRect(
        layoutBounds.m_position.m_x + padding.m_left,
        layoutBounds.m_position.m_y + padding.m_top,
        layoutBounds.m_size.m_x - padding.getHorizontal(),
        layoutBounds.m_size.m_y - padding.getVertical());
    _context.drawText(m_text, textRect, theme.m_typography.m_primary);
}
