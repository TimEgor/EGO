#include "Render.h"

void ego::render::Render::drawPoint(const FloatVector3& _position, const FloatVector4& _color)
{
    DebugDrawPointData point;
    point.m_position = _position;
    point.m_color = _color;
    drawPoint(point);
}

void ego::render::Render::drawLine(
    const FloatVector3& _startPosition,
    const FloatVector3& _endPosition,
    const FloatVector4& _color
)
{
    drawLine(_startPosition, _color, _endPosition, _color);
}

void ego::render::Render::drawLine(
    const FloatVector3& _startPosition,
    const FloatVector4& _startColor,
    const FloatVector3& _endPosition,
    const FloatVector4& _endColor
)
{
    DebugDrawLineData line;
    line.m_start.m_position = _startPosition;
    line.m_start.m_color = _startColor;
    line.m_end.m_position = _endPosition;
    line.m_end.m_color = _endColor;

    drawLine(line);
}
