#include "GuiWindowController.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    constexpr const char* EditorDockSpaceName = "EditorDockSpace";
    constexpr float SceneInspectorWidthRatio = 0.2f;
    constexpr float EntityInspectorWidthRatio = 0.22f;
} // namespace

void ego::editor::GuiWindowController::reset()
{
    m_isViewportVisible = true;
    m_isSceneInspectorVisible = true;
    m_isEntityInspectorVisible = true;
}

float ego::editor::GuiWindowController::drawWindowMenu()
{
    const bool isWindowMenuOpen = ImGui::BeginMenu("Window");
    const float windowMenuMaxX = ImGui::GetItemRectMax().x;
    if (isWindowMenuOpen)
    {
        ImGui::MenuItem("Viewport", nullptr, &m_isViewportVisible);
        ImGui::MenuItem("Scene Inspector", nullptr, &m_isSceneInspectorVisible);
        ImGui::MenuItem("Entity Inspector", nullptr, &m_isEntityInspectorVisible);
        ImGui::EndMenu();
    }

    return windowMenuMaxX;
}

void ego::editor::GuiWindowController::drawWindows(gui::GuiFrameTextureID _sceneTextureID)
{
    drawDockSpace();
    drawViewport(_sceneTextureID);
    drawSceneInspector();
    drawEntityInspector();
}

bool ego::editor::GuiWindowController::isViewportVisible() const
{
    return m_isViewportVisible;
}

void ego::editor::GuiWindowController::drawDockSpace()
{
    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);
    if (!ImGui::DockBuilderGetNode(dockSpaceID))
    {
        const ImGuiViewport* mainViewport = ImGui::GetMainViewport();

        ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceID, mainViewport->WorkSize);

        ImGuiID viewportNodeID = dockSpaceID;
        const ImGuiID sceneInspectorNodeID = ImGui::DockBuilderSplitNode(viewportNodeID, ImGuiDir_Left, SceneInspectorWidthRatio, nullptr, &viewportNodeID);
        const ImGuiID entityInspectorNodeID = ImGui::DockBuilderSplitNode(viewportNodeID, ImGuiDir_Right, EntityInspectorWidthRatio, nullptr, &viewportNodeID);

        ImGui::DockBuilderDockWindow("Viewport", viewportNodeID);
        ImGui::DockBuilderDockWindow("Scene Inspector", sceneInspectorNodeID);
        ImGui::DockBuilderDockWindow("Entity Inspector", entityInspectorNodeID);
        ImGui::DockBuilderFinish(dockSpaceID);
    }

    ImGui::DockSpaceOverViewport(dockSpaceID);
}

void ego::editor::GuiWindowController::drawViewport(gui::GuiFrameTextureID _sceneTextureID)
{
    if (!m_isViewportVisible)
    {
        return;
    }

    bool isViewportVisible = true;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Viewport", &isViewportVisible))
    {
        const ImVec2 availableSize = ImGui::GetContentRegionAvail();
        if (_sceneTextureID != gui::InvalidGuiFrameTextureID && availableSize.x > 0.0f && availableSize.y > 0.0f)
        {
            ImGui::Image(ImTextureRef(static_cast<ImTextureID>(_sceneTextureID)), availableSize);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
    m_isViewportVisible = isViewportVisible;
}

void ego::editor::GuiWindowController::drawSceneInspector()
{
    if (!m_isSceneInspectorVisible)
    {
        return;
    }

    bool isSceneInspectorVisible = true;
    if (ImGui::Begin("Scene Inspector", &isSceneInspectorVisible))
    {
        const ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (ImGui::TreeNodeEx("Scene", sceneFlags))
        {
            const ImGuiTreeNodeFlags cameraFlags =
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth;
            ImGui::TreeNodeEx("Camera", cameraFlags);
            ImGui::TreePop();
        }
    }

    ImGui::End();
    m_isSceneInspectorVisible = isSceneInspectorVisible;
}

void ego::editor::GuiWindowController::drawEntityInspector()
{
    if (!m_isEntityInspectorVisible)
    {
        return;
    }

    bool isEntityInspectorVisible = true;
    if (ImGui::Begin("Entity Inspector", &isEntityInspectorVisible))
    {
        ImGui::TextDisabled("Selection");
        ImGui::SameLine();
        ImGui::TextUnformatted("Camera");
        ImGui::SeparatorText("Camera Component");
    }

    ImGui::End();
    m_isEntityInspectorVisible = isEntityInspectorVisible;
}
