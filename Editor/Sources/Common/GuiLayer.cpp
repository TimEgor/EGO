#include "GuiLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    constexpr const char* EditorDockSpaceName = "EditorDockSpace";
    constexpr float HierarchyWidthRatio = 0.2f;
    constexpr float InspectorWidthRatio = 0.22f;
    constexpr float ConsoleHeightRatio = 0.25f;
} // namespace

void ego::editor::GuiLayer::setSceneTexture(const gpu::TextureViewPointer& _sceneTexture)
{
    m_sceneTexture = _sceneTexture;
}

void ego::editor::GuiLayer::reset()
{
    m_sceneTexture = nullptr;
    m_showSceneWindow = true;
    m_showHierarchyWindow = true;
    m_showInspectorWindow = true;
    m_showConsoleWindow = true;
    m_isDefaultLayoutInitialized = false;
}

void ego::editor::GuiLayer::initializeDefaultLayout()
{
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);

    ImGui::DockBuilderRemoveNode(dockSpaceID);
    ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceID, mainViewport->WorkSize);

    ImGuiID sceneNodeID = dockSpaceID;
    const ImGuiID hierarchyNodeID = ImGui::DockBuilderSplitNode(sceneNodeID, ImGuiDir_Left, HierarchyWidthRatio, nullptr, &sceneNodeID);
    const ImGuiID inspectorNodeID = ImGui::DockBuilderSplitNode(sceneNodeID, ImGuiDir_Right, InspectorWidthRatio, nullptr, &sceneNodeID);
    const ImGuiID consoleNodeID = ImGui::DockBuilderSplitNode(sceneNodeID, ImGuiDir_Down, ConsoleHeightRatio, nullptr, &sceneNodeID);

    ImGui::DockBuilderDockWindow("Scene", sceneNodeID);
    ImGui::DockBuilderDockWindow("Hierarchy", hierarchyNodeID);
    ImGui::DockBuilderDockWindow("Inspector", inspectorNodeID);
    ImGui::DockBuilderDockWindow("Console", consoleNodeID);
    ImGui::DockBuilderFinish(dockSpaceID);

    m_isDefaultLayoutInitialized = true;
}

void ego::editor::GuiLayer::drawGui()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Scene", nullptr, &m_showSceneWindow);
            ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchyWindow);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspectorWindow);
            ImGui::MenuItem("Console", nullptr, &m_showConsoleWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (!m_isDefaultLayoutInitialized)
    {
        initializeDefaultLayout();
    }

    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);
    ImGui::DockSpaceOverViewport(dockSpaceID);

    if (m_showSceneWindow)
    {
        if (ImGui::Begin("Scene", &m_showSceneWindow))
        {
            const ImVec2 availableSize = ImGui::GetContentRegionAvail();
            if (m_sceneTexture && availableSize.x > 0.0f && availableSize.y > 0.0f)
            {
                const gui::GuiFrameTextureID textureID = bindTexture(m_sceneTexture);
                if (textureID != gui::InvalidGuiFrameTextureID)
                {
                    ImGui::Image(ImTextureRef(static_cast<ImTextureID>(textureID)), availableSize);
                }
            }
        }

        ImGui::End();
    }

    if (m_showHierarchyWindow)
    {
        if (ImGui::Begin("Hierarchy", &m_showHierarchyWindow))
        {
            ImGui::BulletText("Scene");
            ImGui::Indent();
            ImGui::BulletText("Camera");
            ImGui::Unindent();
        }

        ImGui::End();
    }

    if (m_showInspectorWindow)
    {
        if (ImGui::Begin("Inspector", &m_showInspectorWindow))
        {
            ImGui::TextUnformatted("Selection: Camera");
            ImGui::Separator();
            ImGui::TextUnformatted("Component: CameraComponent");
        }

        ImGui::End();
    }

    if (m_showConsoleWindow)
    {
        if (ImGui::Begin("Console", &m_showConsoleWindow))
        {
            ImGui::TextUnformatted("EgoEditor is ready.");
            ImGui::TextUnformatted("The scene session is always active.");
            ImGui::TextUnformatted("Static editor test primitives are rendered every frame.");
        }

        ImGui::End();
    }
}
