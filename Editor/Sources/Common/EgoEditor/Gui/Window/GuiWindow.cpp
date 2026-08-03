#include "EgoEditor/Gui/Window/GuiWindow.h"

#include "EgoCore/UtilsMacros.h"

bool ego::editor::GuiWindow::isVisible() const
{
    return m_isVisible;
}

void ego::editor::GuiWindow::setVisible(bool _isVisible)
{
    m_isVisible = _isVisible;
}

void ego::editor::GuiWindow::draw()
{
    EGO_CHECK_RETURN(m_isVisible);

    drawWindow(m_isVisible);
}
