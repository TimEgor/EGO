#include "EgoEditor/Gui/Window/EntityInspectorWindow.h"

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"
#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

#include "EgoGui/GuiController.h"

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

                const engine::EngineSessionPointer engineSession = editorSubsystem->getEditorController().getEditorEngineSessionPointer();
                const gui::GuiControllerPointer guiController = engineSession ? engineSession->getGuiControllerPointer() : nullptr;
                const gui::PropertyInspectorPointer propertyInspector = guiController ? guiController->getPropertyInspectorPointer() : nullptr;
                if (!propertyInspector)
                {
                    ImGui::TextDisabled("Property inspector isn't available");
                }
                else
                {
                    level->forEachComponent(
                        selectedEntity,
                        [this, &propertyInspector](ecs::Component& _component)
                        {
                            drawComponent(_component, *propertyInspector);
                        });
                }
            }
        }
    }

    ImGui::End();
}

void ego::editor::EntityInspectorWindow::drawComponent(ecs::Component& _component, const gui::PropertyInspector& _propertyInspector)
{
    const rtti::TypeMetaInfo& typeMetaInfo = _component.getObjectTypeMetaInfo();
    const rtti::TypeMetaInfo::PropertyRange properties = typeMetaInfo.getProperties();

    ImGui::SeparatorText(_component.getObjectTypeInfoName());
    if (properties.empty())
    {
        return;
    }

    drawProperties(&_component, typeMetaInfo, _propertyInspector);
}

void ego::editor::EntityInspectorWindow::drawProperties(
    void* _object,
    const rtti::TypeMetaInfo& _typeMetaInfo,
    const gui::PropertyInspector& _propertyInspector)
{
    const rtti::TypeMetaInfo::PropertyRange properties = _typeMetaInfo.getProperties();
    for (rtti::TypeMetaInfo::PropertyIterator iterator = properties.begin(); iterator != properties.end(); ++iterator)
    {
        const rtti::PropertyMetaInfo& propertyMetaInfo = *iterator;
        void* propertyValue = iterator.getValueAddress(_object);

        drawProperty(propertyMetaInfo.m_name, propertyValue, propertyMetaInfo, _propertyInspector);
    }
}

void ego::editor::EntityInspectorWindow::drawProperty(
    const char* _name,
    void* _value,
    const rtti::PropertyMetaInfo& _propertyMetaInfo,
    const gui::PropertyInspector& _propertyInspector)
{
    const gui::PropertyGuiDrawerPointer propertyGuiDrawer = _propertyInspector.getPropertyGuiDrawerPointer(_propertyMetaInfo);

    ImGui::PushID(&_propertyMetaInfo);
    if (propertyGuiDrawer)
    {
        const gui::PropertyGuiDrawFunction drawPropertyFunction =
            [this, &_propertyInspector](const char* _childName, void* _childValue, const rtti::PropertyMetaInfo& _childPropertyMetaInfo)
        {
            drawProperty(_childName, _childValue, _childPropertyMetaInfo, _propertyInspector);
        };

        propertyGuiDrawer->draw(_name, _value, _propertyMetaInfo, drawPropertyFunction);
    }
    else if (_propertyMetaInfo.m_valueTypeMetaInfo && ImGui::TreeNode(_name))
    {
        const rtti::TypeMetaInfo::PropertyRange childProperties = _propertyMetaInfo.m_valueTypeMetaInfo->getProperties();
        if (childProperties.empty())
        {
            ImGui::TextDisabled("No reflected properties");
        }
        else
        {
            drawProperties(_value, *_propertyMetaInfo.m_valueTypeMetaInfo, _propertyInspector);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}
