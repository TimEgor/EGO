#include "GuiTypes.h"

float ego::gui::GuiRect::getLeft() const
{
    return m_position.m_x;
}

float ego::gui::GuiRect::getTop() const
{
    return m_position.m_y;
}

float ego::gui::GuiRect::getRight() const
{
    return m_position.m_x + m_size.m_x;
}

float ego::gui::GuiRect::getBottom() const
{
    return m_position.m_y + m_size.m_y;
}

bool ego::gui::GuiRect::contains(const GuiPosition& _position) const
{
    return _position.m_x >= getLeft() && _position.m_x <= getRight() && _position.m_y >= getTop() && _position.m_y <= getBottom();
}

float ego::gui::GuiMargin::getHorizontal() const
{
    return m_left + m_right;
}

float ego::gui::GuiMargin::getVertical() const
{
    return m_top + m_bottom;
}
