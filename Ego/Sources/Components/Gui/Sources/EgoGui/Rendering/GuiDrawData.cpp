#include "GuiDrawData.h"

void ego::gui::GuiDrawData::clear()
{
    m_viewportSize = GuiSizeZero;
    m_vertices.clear();
    m_indices.clear();
    m_commands.clear();
}

bool ego::gui::GuiDrawData::isEmpty() const
{
    return m_commands.empty();
}
