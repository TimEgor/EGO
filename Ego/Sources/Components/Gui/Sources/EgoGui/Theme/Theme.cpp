#include "Theme.h"

const ego::NormalizedColorRGBA& ego::gui::InteractionColors::resolve(bool _isHovered, bool _isPressed) const
{
    if (_isPressed)
    {
        return m_pressed;
    }
    if (_isHovered)
    {
        return m_hovered;
    }
    return m_normal;
}

const ego::gui::Theme& ego::gui::Theme::GetDefault()
{
    static const Theme DefaultTheme;
    return DefaultTheme;
}
