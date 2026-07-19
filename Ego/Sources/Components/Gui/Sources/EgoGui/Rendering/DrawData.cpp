#include "DrawData.h"

void ego::gui::DrawData::clear()
{
    m_viewportSize = SizeZero;
    m_vertices.clear();
    m_indices.clear();
    m_commands.clear();
}

bool ego::gui::DrawData::isEmpty() const
{
    return m_commands.empty();
}
