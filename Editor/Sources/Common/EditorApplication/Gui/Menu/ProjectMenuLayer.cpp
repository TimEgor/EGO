#include "ProjectMenuLayer.h"

#include "EgoCore/UtilsMacros.h"

#include "EditorApplication/EditorProjectController.h"

#include <imgui.h>

ego::editor::ProjectMenuLayer::ProjectMenuLayer(EditorProjectController& _projectController)
    : m_projectController(_projectController)
{
}

float ego::editor::ProjectMenuLayer::drawMenu()
{
    const bool isMenuOpen = ImGui::BeginMenu("Project");
    const float menuMaxX = ImGui::GetItemRectMax().x;
    EGO_CHECK_RETURN_VALUE(isMenuOpen, menuMaxX);

    if (ImGui::MenuItem("Load Project..."))
    {
        m_projectController.loadProject();
    }

    if (m_projectController.isProjectLoaded() && ImGui::MenuItem("Unload Project"))
    {
        m_projectController.unloadProject();
    }

    ImGui::EndMenu();

    return menuMaxX;
}
