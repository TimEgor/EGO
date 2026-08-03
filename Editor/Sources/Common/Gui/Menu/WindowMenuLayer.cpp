#include "WindowMenuLayer.h"

#include "EgoCore/UtilsMacros.h"

#include "EditorController.h"
#include "EditorSubsystem.h"
#include "Gui/Window/GuiWindowController.h"

#include <imgui.h>

float ego::editor::WindowMenuLayer::drawMenu()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_RETURN_VALUE(editorSubsystem, ImGui::GetCursorScreenPos().x);

    const bool isMenuOpen = ImGui::BeginMenu("Window");
    const float menuMaxX = ImGui::GetItemRectMax().x;
    EGO_CHECK_RETURN_VALUE(isMenuOpen, menuMaxX);

    const GuiWindowController& windowController = editorSubsystem->getEditorController().getGuiController().getWindowController();

    const GuiWindowPointer viewportWindow = windowController.getViewportWindowPointer();
    if (viewportWindow)
    {
        const bool isVisible = viewportWindow->isVisible();
        if (ImGui::MenuItem("Viewport", nullptr, isVisible))
        {
            viewportWindow->setVisible(!isVisible);
        }
    }

    const GuiWindowPointer sceneInspectorWindow = windowController.getSceneInspectorWindowPointer();
    if (sceneInspectorWindow)
    {
        const bool isVisible = sceneInspectorWindow->isVisible();
        if (ImGui::MenuItem("Scene Inspector", nullptr, isVisible))
        {
            sceneInspectorWindow->setVisible(!isVisible);
        }
    }

    const GuiWindowPointer entityInspectorWindow = windowController.getEntityInspectorWindowPointer();
    if (entityInspectorWindow)
    {
        const bool isVisible = entityInspectorWindow->isVisible();
        if (ImGui::MenuItem("Entity Inspector", nullptr, isVisible))
        {
            entityInspectorWindow->setVisible(!isVisible);
        }
    }

    ImGui::EndMenu();

    return menuMaxX;
}
