#include "ProjectMenuLayer.h"

#include "EgoCore/UtilsMacros.h"

#include "EditorApplication/EditorController.h"
#include "EditorApplication/EditorProjectController.h"
#include "EditorApplication/EditorSubsystem.h"

#include <imgui.h>

float ego::editor::ProjectMenuLayer::drawMenu()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN_VALUE(editorSubsystem, ImGui::GetCursorScreenPos().x);

    EditorProjectController& projectController = editorSubsystem->getEditorController().getProjectController();

    const bool isMenuOpen = ImGui::BeginMenu("Project");
    const float menuMaxX = ImGui::GetItemRectMax().x;
    EGO_CHECK_RETURN_VALUE(isMenuOpen, menuMaxX);

    if (ImGui::MenuItem("Create Project..."))
    {
        projectController.createProject();
    }

    if (ImGui::MenuItem("Load Project..."))
    {
        projectController.loadProject();
    }

    if (projectController.isProjectLoaded() && ImGui::MenuItem("Save Project"))
    {
        projectController.saveProject();
    }

    if (projectController.isProjectLoaded() && ImGui::MenuItem("Unload Project"))
    {
        projectController.unloadProject();
    }

    ImGui::EndMenu();

    return menuMaxX;
}
