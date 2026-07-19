#include "Geometry.h"

float ego::gui::Rect::getLeft() const
{
    return m_position.m_x;
}

float ego::gui::Rect::getTop() const
{
    return m_position.m_y;
}

float ego::gui::Rect::getRight() const
{
    return m_position.m_x + m_size.m_x;
}

float ego::gui::Rect::getBottom() const
{
    return m_position.m_y + m_size.m_y;
}

bool ego::gui::Rect::contains(const Position& _position) const
{
    return m_size.m_x > 0.0f && m_size.m_y > 0.0f && _position.m_x >= getLeft() && _position.m_x < getRight() && _position.m_y >= getTop() &&
           _position.m_y < getBottom();
}
