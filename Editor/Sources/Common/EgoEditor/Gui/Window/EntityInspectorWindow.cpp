#include "EgoEditor/Gui/Window/EntityInspectorWindow.h"

#include "EgoEngine/Graphic/SceneRender/Component/CameraComponent.h"
#include "EgoEngine/Graphic/SceneRender/Component/MeshRenderComponent.h"
#include "EgoEngine/Level/Level.h"

#include "EgoEditor/EditorController.h"
#include "EgoEditor/EditorSubsystem.h"
#include "EgoEditor/ProjectController.h"

#include <imgui.h>

namespace
{
    constexpr std::string_view WindowTitle = "Entity Inspector";
} // namespace

std::string_view ego::editor::EntityInspectorWindow::getTitle() const
{
    return WindowTitle;
}

void ego::editor::EntityInspectorWindow::drawWindow(bool& _isVisible)
{
    if (ImGui::Begin(WindowTitle.data(), &_isVisible))
    {
        const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
        if (!editorSubsystem)
        {
            ImGui::TextDisabled("No entity selected");
        }
        else
        {
            ProjectController& projectController = editorSubsystem->getEditorController().getProjectController();
            const LevelPointer level = projectController.getCurrentLevelPointer();
            const ecs::Entity selectedEntity = projectController.getSelectedEntity();
            if (!level || !selectedEntity)
            {
                ImGui::TextDisabled("No entity selected");
            }
            else
            {
                const NameComponent* nameComponent = level->tryGetComponent<NameComponent>(selectedEntity);
                const char* entityName = nameComponent && !nameComponent->m_name.empty() ? nameComponent->m_name.c_str() : "Entity";

                ImGui::TextDisabled("Selection");
                ImGui::SameLine();
                ImGui::TextUnformatted(entityName);
                ImGui::Text("Entity ID: %u", static_cast<unsigned int>(selectedEntity.getID()));

                const ecs::Entity parentEntity = level->getNodeParent(selectedEntity);
                if (parentEntity)
                {
                    ImGui::Text("Parent ID: %u", static_cast<unsigned int>(parentEntity.getID()));
                }
                else
                {
                    ImGui::TextUnformatted("Parent: Root");
                }

                if (nameComponent)
                {
                    ImGui::SeparatorText("Name Component");
                    ImGui::TextUnformatted(nameComponent->m_name.c_str());
                }

                const TransformComponent* transformComponent = level->tryGetComponent<TransformComponent>(selectedEntity);
                if (transformComponent)
                {
                    const FloatVector3 position = transformComponent->m_globalTransform.getOrigin().getFloatVector3();

                    ImGui::SeparatorText("Transform Component");
                    ImGui::Text("Position: %.3f, %.3f, %.3f", position.m_x, position.m_y, position.m_z);
                }

                const render::CameraComponent* cameraComponent = level->tryGetComponent<render::CameraComponent>(selectedEntity);
                if (cameraComponent)
                {
                    ImGui::SeparatorText("Camera Component");
                }

                const render::MeshRenderComponent* meshRenderComponent = level->tryGetComponent<render::MeshRenderComponent>(selectedEntity);
                if (meshRenderComponent)
                {
                    ImGui::SeparatorText("Mesh Render Component");
                    ImGui::Text("Mesh: %s", meshRenderComponent->m_mesh ? "Assigned" : "None");
                    ImGui::Text("Material: %s", meshRenderComponent->m_material ? "Assigned" : "None");
                }
            }
        }
    }

    ImGui::End();
}
