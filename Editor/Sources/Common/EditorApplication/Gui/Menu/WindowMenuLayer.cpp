#include "WindowMenuLayer.h"

#include "EgoCore/UtilsMacros.h"

#include "EditorApplication/Gui/GuiWindowController.h"

#include <imgui.h>

ego::editor::WindowMenuLayer::WindowMenuLayer(GuiWindowController& _windowController)
    : m_windowController(_windowController)
{
}

float ego::editor::WindowMenuLayer::drawMenu()
{
    const bool isMenuOpen = ImGui::BeginMenu("Window");
    const float menuMaxX = ImGui::GetItemRectMax().x;
    EGO_CHECK_RETURN_VALUE(isMenuOpen, menuMaxX);

    const bool isViewportVisible = m_windowController.isViewportVisible();
    if (ImGui::MenuItem("Viewport", nullptr, isViewportVisible))
    {
        m_windowController.setViewportVisible(!isViewportVisible);
    }

    const bool isSceneInspectorVisible = m_windowController.isSceneInspectorVisible();
    if (ImGui::MenuItem("Scene Inspector", nullptr, isSceneInspectorVisible))
    {
        m_windowController.setSceneInspectorVisible(!isSceneInspectorVisible);
    }

    const bool isEntityInspectorVisible = m_windowController.isEntityInspectorVisible();
    if (ImGui::MenuItem("Entity Inspector", nullptr, isEntityInspectorVisible))
    {
        m_windowController.setEntityInspectorVisible(!isEntityInspectorVisible);
    }

    ImGui::EndMenu();

    return menuMaxX;
}
