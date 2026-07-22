#include "Geometry.h"

bool ego::gui::AreEqual(const FloatVector2& _first, const FloatVector2& _second)
{
    return _first.m_x == _second.m_x && _first.m_y == _second.m_y;
}

bool ego::gui::Rect::operator==(const Rect& _rect) const
{
    return AreEqual(m_position, _rect.m_position) && AreEqual(m_size, _rect.m_size);
}

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
