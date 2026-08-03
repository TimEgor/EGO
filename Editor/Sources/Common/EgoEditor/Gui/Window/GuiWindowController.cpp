#include "EgoEditor/Gui/Window/GuiWindowController.h"

#include <string>
#include <string_view>

#include "EgoCore/UtilsMacros.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    constexpr const char* EditorDockSpaceName = "EditorDockSpace";
    constexpr std::string_view ModalWindowIDSuffix = "###EditorModalWindow";
    constexpr float SceneInspectorWidthRatio = 0.2f;
    constexpr float EntityInspectorWidthRatio = 0.22f;
} // namespace

bool ego::editor::GuiWindowController::init()
{
    release();

    EGO_CHECK_INITIALIZATION(initWindows());

    return true;
}

void ego::editor::GuiWindowController::release()
{
    releaseWindows();

    m_modalWindowStack.clear();
    m_nextModalWindowInstanceID = 0;
}

ego::editor::ViewportWindowPointer ego::editor::GuiWindowController::getViewportWindowPointer() const
{
    return m_viewportWindow;
}

ego::editor::SceneInspectorWindowPointer ego::editor::GuiWindowController::getSceneInspectorWindowPointer() const
{
    return m_sceneInspectorWindow;
}

ego::editor::EntityInspectorWindowPointer ego::editor::GuiWindowController::getEntityInspectorWindowPointer() const
{
    return m_entityInspectorWindow;
}

bool ego::editor::GuiWindowController::pushModalWindow(const GuiModalWindowPointer& _window)
{
    EGO_CHECK_RETURN_FALSE(_window);

    m_modalWindowStack.push_back({.m_instanceID = m_nextModalWindowInstanceID, .m_window = _window});
    ++m_nextModalWindowInstanceID;

    return true;
}

void ego::editor::GuiWindowController::drawWindows()
{
    drawDockSpace();

    for (const GuiWindowPointer& window : m_windows)
    {
        if (window)
        {
            window->draw();
        }
    }

    EGO_CHECK_RETURN(!m_modalWindowStack.empty());

    drawModalWindow(0);
}

bool ego::editor::GuiWindowController::initWindows()
{
    m_viewportWindow = MakePointer<ViewportWindow>();
    m_sceneInspectorWindow = MakePointer<SceneInspectorWindow>();
    m_entityInspectorWindow = MakePointer<EntityInspectorWindow>();
    EGO_CHECK_RETURN_CALL_FALSE(m_viewportWindow && m_sceneInspectorWindow && m_entityInspectorWindow, releaseWindows());

    m_windows.push_back(m_viewportWindow);
    m_windows.push_back(m_sceneInspectorWindow);
    m_windows.push_back(m_entityInspectorWindow);

    return true;
}

void ego::editor::GuiWindowController::releaseWindows()
{
    m_windows.clear();

    m_viewportWindow = nullptr;
    m_sceneInspectorWindow = nullptr;
    m_entityInspectorWindow = nullptr;
}

void ego::editor::GuiWindowController::drawDockSpace()
{
    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);
    if (!ImGui::DockBuilderGetNode(dockSpaceID))
    {
        restoreDefaultDockLayout();
    }

    ImGui::DockSpaceOverViewport(dockSpaceID);
}

void ego::editor::GuiWindowController::restoreDefaultDockLayout()
{
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    EGO_CHECK_RETURN(mainViewport);

    const ImGuiID dockSpaceID = ImHashStr(EditorDockSpaceName);
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

void ego::editor::GuiWindowController::drawModalWindow(std::size_t _index)
{
    EGO_CHECK_RETURN(_index < m_modalWindowStack.size());

    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    EGO_CHECK_RETURN(mainViewport);

    const ModalWindowRecord windowRecord = m_modalWindowStack[_index];
    EGO_CHECK_RETURN(windowRecord.m_window);

    std::string windowName(windowRecord.m_window->getTitle());
    windowName += ModalWindowIDSuffix;
    windowName += std::to_string(windowRecord.m_instanceID);

    if (!ImGui::IsPopupOpen(windowName.c_str()))
    {
        ImGui::OpenPopup(windowName.c_str());
    }

    ImGuiViewport* parentViewport = _index > 0 ? ImGui::GetWindowViewport() : mainViewport;
    EGO_CHECK_RETURN(parentViewport);

    ImGuiWindowClass windowClass;
    windowClass.ParentViewportId = parentViewport->ID;
    windowClass.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
    ImGui::SetNextWindowClass(&windowClass);

    if (!ImGui::BeginPopupModal(windowName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
    {
        return;
    }

    const bool isOpen = windowRecord.m_window->draw();
    if (!isOpen && _index + 1 == m_modalWindowStack.size() && m_modalWindowStack.back().m_instanceID == windowRecord.m_instanceID)
    {
        m_modalWindowStack.pop_back();
        ImGui::CloseCurrentPopup();
    }
    else if (_index + 1 < m_modalWindowStack.size())
    {
        drawModalWindow(_index + 1);
    }

    ImGui::EndPopup();
}
