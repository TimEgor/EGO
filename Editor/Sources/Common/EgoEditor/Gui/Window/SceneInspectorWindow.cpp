#include "EgoEditor/Gui/Window/SceneInspectorWindow.h"

#include "EgoEngine/Level/Level.h"

#include "EgoEditor/EditorController.h"
#include "EgoEditor/EditorSubsystem.h"
#include "EgoEditor/ProjectController.h"

#include <imgui.h>

namespace
{
    constexpr std::string_view WindowTitle = "Scene Inspector";
} // namespace

std::string_view ego::editor::SceneInspectorWindow::getTitle() const
{
    return WindowTitle;
}

void ego::editor::SceneInspectorWindow::drawWindow(bool& _isVisible)
{
    if (ImGui::Begin(WindowTitle.data(), &_isVisible))
    {
        const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
        if (!editorSubsystem)
        {
            ImGui::TextDisabled("No current level");
        }
        else
        {
            ProjectController& projectController = editorSubsystem->getEditorController().getProjectController();
            const LevelPointer level = projectController.getCurrentLevelPointer();
            if (!level)
            {
                ImGui::TextDisabled("No current level");
            }
            else
            {
                const ecs::Entity selectedEntity = projectController.getSelectedEntity();
                bool hasRootNodes = false;
                for (ecs::Entity rootNode : level->getRootNodes())
                {
                    hasRootNodes = true;
                    drawNode(projectController, *level, rootNode, selectedEntity);
                }

                if (!hasRootNodes)
                {
                    ImGui::TextDisabled("Level hierarchy is empty");
                }

                if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
                {
                    projectController.clearSelectedEntity();
                }
            }
        }
    }

    ImGui::End();
}

void ego::editor::SceneInspectorWindow::drawNode(
    ProjectController& _projectController,
    const Level& _level,
    ecs::Entity _node,
    ecs::Entity _selectedEntity)
{
    if (!_level.isNode(_node))
    {
        return;
    }

    const NameComponent* nameComponent = _level.tryGetComponent<NameComponent>(_node);
    const char* nodeName = nameComponent && !nameComponent->m_name.empty() ? nameComponent->m_name.c_str() : "Entity";
    const bool hasChildren = static_cast<bool>(_level.getFirstNodeChild(_node));

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (_node == _selectedEntity)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!hasChildren)
    {
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    ImGui::PushID(static_cast<int>(_node.getWorldID()));
    ImGui::PushID(static_cast<int>(_node.getID()));
    const bool isOpen = ImGui::TreeNodeEx("Node", nodeFlags, "%s", nodeName);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        _projectController.selectEntity(_node);
    }

    if (hasChildren && isOpen)
    {
        for (ecs::Entity childNode : _level.getNodeChildren(_node))
        {
            drawNode(_projectController, _level, childNode, _selectedEntity);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::PopID();
}
