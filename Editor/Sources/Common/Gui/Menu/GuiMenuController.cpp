#include "GuiMenuController.h"

#include "EgoCore/UtilsMacros.h"

#include "ProjectMenuLayer.h"
#include "WindowMenuLayer.h"

#include <imgui.h>

bool ego::editor::GuiMenuController::init()
{
    release();

    m_projectMenuLayer = MakePointer<ProjectMenuLayer>();
    m_windowMenuLayer = MakePointer<WindowMenuLayer>();
    EGO_CHECK_INITIALIZATION(m_projectMenuLayer && m_windowMenuLayer);

    return true;
}

void ego::editor::GuiMenuController::release()
{
    m_projectMenuLayer = nullptr;
    m_windowMenuLayer = nullptr;
}

float ego::editor::GuiMenuController::draw()
{
    float menuMaxX = ImGui::GetCursorScreenPos().x;

    if (m_projectMenuLayer)
    {
        menuMaxX = std::max(m_projectMenuLayer->drawMenu(), menuMaxX);
    }

    if (m_windowMenuLayer)
    {
        menuMaxX = std::max(m_windowMenuLayer->drawMenu(), menuMaxX);
    }

    return menuMaxX;
}
