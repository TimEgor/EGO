#include "GuiLayer.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    constexpr const char* EditorDockSpaceName = "EditorDockSpace";
    constexpr float SceneInspectorWidthRatio = 0.2f;
    constexpr float EntityInspectorWidthRatio = 0.22f;
} // namespace

void ego::editor::GuiLayer::setSurface(const PlatformSurfacePointer& _surface)
{
    m_surface = _surface;
}

void ego::editor::GuiLayer::setSceneTexture(const gpu::TextureViewPointer& _sceneTexture)
{
    m_sceneTexture = _sceneTexture;
}

void ego::editor::GuiLayer::reset()
{
    m_surface = nullptr;
    m_sceneTexture = nullptr;
    m_showViewport = true;
    m_showSceneInspector = true;
    m_showEntityInspector = true;
    m_isDefaultLayoutInitialized = false;
}

void ego::editor::GuiLayer::initializeDefaultLayout()
{
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);

    ImGui::DockBuilderRemoveNode(dockSpaceID);
    ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockSpaceID, mainViewport->WorkSize);

    ImGuiID viewportNodeID = dockSpaceID;
    const ImGuiID sceneInspectorNodeID = ImGui::DockBuilderSplitNode(viewportNodeID, ImGuiDir_Left, SceneInspectorWidthRatio, nullptr, &viewportNodeID);
    const ImGuiID entityInspectorNodeID = ImGui::DockBuilderSplitNode(viewportNodeID, ImGuiDir_Right, EntityInspectorWidthRatio, nullptr, &viewportNodeID);

    ImGui::DockBuilderDockWindow("Viewport", viewportNodeID);
    ImGui::DockBuilderDockWindow("Scene Inspector", sceneInspectorNodeID);
    ImGui::DockBuilderDockWindow("Entity Inspector", entityInspectorNodeID);
    ImGui::DockBuilderFinish(dockSpaceID);

    m_isDefaultLayoutInitialized = true;
}

void ego::editor::GuiLayer::drawGui()
{
    if (m_surface)
    {
        m_titleBar.draw(*m_surface, m_showViewport, m_showSceneInspector, m_showEntityInspector);
    }

    if (!m_isDefaultLayoutInitialized)
    {
        initializeDefaultLayout();
    }

    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);
    ImGui::DockSpaceOverViewport(dockSpaceID);

    if (m_showViewport)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Viewport", &m_showViewport))
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
        ImGui::PopStyleVar();
    }

    if (m_showSceneInspector)
    {
        if (ImGui::Begin("Scene Inspector", &m_showSceneInspector))
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
    }

    if (m_showEntityInspector)
    {
        if (ImGui::Begin("Entity Inspector", &m_showEntityInspector))
        {
            ImGui::TextDisabled("Selection");
            ImGui::SameLine();
            ImGui::TextUnformatted("Camera");
            ImGui::SeparatorText("Camera Component");
        }

        ImGui::End();
    }
}
