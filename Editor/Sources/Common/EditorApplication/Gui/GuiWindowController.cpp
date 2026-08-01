#include "GuiWindowController.h"

#include "EgoCore/UtilsMacros.h"

#include "EditorApplication/EditorController.h"
#include "EditorApplication/EditorSubsystem.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    constexpr const char* EditorDockSpaceName = "EditorDockSpace";
    constexpr float SceneInspectorWidthRatio = 0.2f;
    constexpr float EntityInspectorWidthRatio = 0.22f;
} // namespace

bool ego::editor::GuiWindowController::init()
{
    if (m_menuLayer)
    {
        return true;
    }

    m_menuLayer = MakePointer<WindowMenuLayer>(*this);
    EGO_CHECK_INITIALIZATION(m_menuLayer);

    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    EGO_CHECK_INITIALIZATION(editorSubsystem);

    EGO_CHECK_INITIALIZATION(editorSubsystem->getEditorController().getEditorGuiController().registerMenuLayer(m_menuLayer, GuiMenuOrder::Window));

    return true;
}

void ego::editor::GuiWindowController::release()
{
    if (m_menuLayer)
    {
        const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
        if (editorSubsystem)
        {
            editorSubsystem->getEditorController().getEditorGuiController().unregisterMenuLayer(m_menuLayer);
        }
    }

    m_menuLayer = nullptr;
    m_isViewportVisible = true;
    m_isSceneInspectorVisible = true;
    m_isEntityInspectorVisible = true;
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

void ego::editor::GuiWindowController::setViewportVisible(bool _isVisible)
{
    m_isViewportVisible = _isVisible;
}

bool ego::editor::GuiWindowController::isSceneInspectorVisible() const
{
    return m_isSceneInspectorVisible;
}

void ego::editor::GuiWindowController::setSceneInspectorVisible(bool _isVisible)
{
    m_isSceneInspectorVisible = _isVisible;
}

bool ego::editor::GuiWindowController::isEntityInspectorVisible() const
{
    return m_isEntityInspectorVisible;
}

void ego::editor::GuiWindowController::setEntityInspectorVisible(bool _isVisible)
{
    m_isEntityInspectorVisible = _isVisible;
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
    EGO_CHECK_RETURN(m_isViewportVisible);

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
    EGO_CHECK_RETURN(m_isSceneInspectorVisible);

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
    EGO_CHECK_RETURN(m_isEntityInspectorVisible);

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
