#include "CheckBox.h"

#include <algorithm>
#include <utility>

#include "EgoGui/Input/Input.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::CheckBoxPointer ego::gui::CheckBox::Create()
{
    return new CheckBox();
}

ego::gui::CheckBoxPointer ego::gui::CheckBox::Create(std::string _text, bool _isChecked, CheckedChangedHandler _onChanged)
{
    const CheckBoxPointer checkBox = new CheckBox();
    checkBox->setText(std::move(_text));
    checkBox->setChecked(_isChecked);
    checkBox->onChange(std::move(_onChanged));
    return checkBox;
}

void ego::gui::CheckBox::setText(std::string _text)
{
    if (m_text == _text)
    {
        return;
    }

    m_text = std::move(_text);
    invalidateLayout();
}

const std::string& ego::gui::CheckBox::getText() const
{
    return m_text;
}

void ego::gui::CheckBox::setChecked(bool _isChecked)
{
    if (m_isChecked == _isChecked)
    {
        return;
    }

    m_isChecked = _isChecked;
}

bool ego::gui::CheckBox::isChecked() const
{
    return m_isChecked;
}

void ego::gui::CheckBox::onChange(CheckedChangedHandler _handler)
{
    m_onCheckedChanged.set(std::move(_handler));
}

void ego::gui::CheckBox::applyUserChecked(bool _isChecked)
{
    if (m_isChecked == _isChecked)
    {
        return;
    }

    m_isChecked = _isChecked;
    m_onCheckedChanged.invoke(m_isChecked);
}

ego::gui::InputReply ego::gui::CheckBox::onPointerMove(InputContext&, const PointerMoveEvent& _event)
{
    m_isHovered = getLayoutBounds().contains(_event.m_position);
    return InputReply::Unhandled;
}

ego::gui::InputReply ego::gui::CheckBox::onMouseButton(InputContext&, const MouseButtonEvent& _event)
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
            applyUserChecked(!m_isChecked);
        }
        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::CheckBox::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_isHovered = getLayoutBounds().contains(_position);
}

void ego::gui::CheckBox::onPointerLeave(const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::CheckBox::onPointerCaptureLost(const Position&)
{
    m_isHovered = false;
    m_isPressed = false;
}

ego::gui::Size ego::gui::CheckBox::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints&)
{
    const SelectionStyle& selection = _context.getTheme().m_selection;
    const float indicatorSize = (std::max)(0.0f, selection.m_indicatorSize);
    const float textOffset = indicatorSize + (std::max)(0.0f, selection.m_labelSpacing);
    const FontAtlasPointer& fontAtlas = _context.getFontAtlas();
    const Size textSize = fontAtlas ? fontAtlas->measureText(m_text) : SizeZero;
    const float minimumHeight = (std::max)(indicatorSize, (std::max)(0.0f, selection.m_minimumHeight));
    return Size(textOffset + textSize.m_x, (std::max)(minimumHeight, textSize.m_y));
}

void ego::gui::CheckBox::drawBaseLayer(PaintContext& _context) const
{
    const Theme& theme = _context.getTheme();
    const SelectionStyle& selection = theme.m_selection;
    const float indicatorSize = (std::max)(0.0f, selection.m_indicatorSize);
    const float markInset = (std::max)(0.0f, selection.m_checkMarkInset);
    const Rect& layoutBounds = getLayoutBounds();
    const Rect boxRect(layoutBounds.m_position, Size(indicatorSize, indicatorSize));
    const NormalizedColorRGBA& boxColor = selection.m_indicator.resolve(m_isHovered, m_isPressed);

    _context.drawBox(boxRect, boxColor);

    if (m_isChecked)
    {
        const float markSize = (std::max)(0.0f, indicatorSize - markInset * 2.0f);
        const Rect markRect(boxRect.m_position.m_x + markInset, boxRect.m_position.m_y + markInset, markSize, markSize);
        _context.drawBox(markRect, selection.m_accent);
    }

    const float textOffset = indicatorSize + (std::max)(0.0f, selection.m_labelSpacing);
    const Rect textRect(layoutBounds.m_position.m_x + textOffset, layoutBounds.m_position.m_y, layoutBounds.m_size.m_x - textOffset, layoutBounds.m_size.m_y);
    _context.drawText(m_text, textRect, theme.m_typography.m_primary);
}
