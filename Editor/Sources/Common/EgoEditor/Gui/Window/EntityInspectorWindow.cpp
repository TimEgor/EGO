#include "EgoEditor/Gui/Window/EntityInspectorWindow.h"

#include <cfloat>
#include <string>

#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

#include "EgoEngine/Level/Level.h"

#include "EgoGui/Inspector/PropertyGui.h"

#include "EgoEditor/EditorController.h"
#include "EgoEditor/EditorSubsystem.h"
#include "EgoEditor/Gui/Window/EntityInspectorWindowStyle.h"
#include "EgoEditor/ProjectController.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::string_view ego::editor::EntityInspectorWindow::getTitle() const
{
    return entity_inspector_style::WindowTitle;
}

void ego::editor::EntityInspectorWindow::drawWindow(bool& _isVisible)
{
    if (ImGui::Begin(entity_inspector_style::WindowTitle.data(), &_isVisible))
    {
        drawContent();
    }

    ImGui::End();
}

void ego::editor::EntityInspectorWindow::drawContent()
{
    const EditorSubsystemPointer editorSubsystem = GetEditorSubsystemPointer();
    if (!editorSubsystem)
    {
        ImGui::TextDisabled("No entity selected");

        return;
    }

    drawSelectedEntity(editorSubsystem->getEditorController());
}

void ego::editor::EntityInspectorWindow::drawSelectedEntity(EditorController& _editorController)
{
    ProjectController& projectController = _editorController.getProjectController();
    const LevelPointer level = projectController.getCurrentLevelPointer();
    const ecs::Entity selectedEntity = projectController.getSelectedEntity();
    if (!level || !selectedEntity)
    {
        ImGui::TextDisabled("No entity selected");

        return;
    }

    drawEntityHeader(*level, selectedEntity);
    ImGui::Spacing();
    drawComponents(*level, selectedEntity);
}

void ego::editor::EntityInspectorWindow::drawEntityHeader(Level& _level, ecs::Entity _entity)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, entity_inspector_style::EntityHeaderColor);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, entity_inspector_style::EntityHeaderRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, entity_inspector_style::EntityHeaderPadding);
    if (ImGui::BeginChild(
            "Entity Header",
            ImVec2(0.0f, 0.0f),
            ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        drawEntityName(_level, _entity);
        ImGui::Spacing();
        drawEntityMetadata(_level, _entity);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void ego::editor::EntityInspectorWindow::drawEntityName(Level& _level, ecs::Entity _entity)
{
    NameComponent* nameComponent = _level.tryGetComponent<NameComponent>(_entity);
    const std::string currentEntityName = nameComponent ? nameComponent->m_name : "Entity";
    std::string editedEntityName = currentEntityName;

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##EntityName", &editedEntityName);
    if (!ImGui::IsItemDeactivatedAfterEdit() || editedEntityName == currentEntityName)
    {
        return;
    }

    if (nameComponent)
    {
        nameComponent->m_name = editedEntityName;
    }
    else
    {
        _level.addOrReplaceComponent<NameComponent>(_entity, NameComponent{editedEntityName});
    }
}

void ego::editor::EntityInspectorWindow::drawEntityMetadata(const Level& _level, ecs::Entity _entity)
{
    if (!ImGui::BeginTable("Entity Metadata", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings))
    {
        return;
    }

    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, entity_inspector_style::MetadataLabelWidth);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("Entity ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%u", static_cast<unsigned int>(_entity.getID()));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextDisabled("Parent");
    ImGui::TableSetColumnIndex(1);

    const ecs::Entity parentEntity = _level.getNodeParent(_entity);
    const NameComponent* parentNameComponent = parentEntity ? _level.tryGetComponent<NameComponent>(parentEntity) : nullptr;
    if (parentNameComponent && !parentNameComponent->m_name.empty())
    {
        ImGui::Text("%s  (%u)", parentNameComponent->m_name.c_str(), static_cast<unsigned int>(parentEntity.getID()));
    }
    else if (parentEntity)
    {
        ImGui::Text("Entity %u", static_cast<unsigned int>(parentEntity.getID()));
    }
    else
    {
        ImGui::TextDisabled("Root");
    }

    ImGui::EndTable();
}

void ego::editor::EntityInspectorWindow::drawComponents(Level& _level, ecs::Entity _entity)
{
    bool hasComponents = false;
    ecs::ComponentTypeID componentToRemove = ecs::InvalidComponentTypeID;
    _level.forEachComponent(
        _entity,
        [this, &hasComponents, &componentToRemove](ecs::ComponentTypeID _componentTypeID, ecs::Component& _component)
        {
            const rtti::TypeMetaInfoID componentTypeMetaInfoID = _component.getObjectTypeMetaInfoID();
            if (!IsComponentVisible(componentTypeMetaInfoID))
            {
                return;
            }

            hasComponents = true;
            if (drawComponent(_component))
            {
                componentToRemove = _componentTypeID;
            }
        });

    if (componentToRemove != ecs::InvalidComponentTypeID)
    {
        _level.removeComponent(_entity, componentToRemove);
    }

    if (!hasComponents)
    {
        ImGui::TextDisabled("No components");
    }

    drawAddComponentButton();
}

bool ego::editor::EntityInspectorWindow::drawComponent(ecs::Component& _component)
{
    const rtti::TypeMetaInfoID componentTypeMetaInfoID = _component.getObjectTypeMetaInfoID();
    const std::string componentName = gui::GetPropertyDisplayName(_component.getObjectTypeInfoName(), "Component");

    ImGui::PushID(&_component);
    const ComponentHeaderState headerState = drawComponentHeader(componentName.c_str(), IsComponentRemovable(componentTypeMetaInfoID));
    if (headerState.m_isExpanded && !headerState.m_shouldRemove)
    {
        drawComponentProperties(_component);
    }

    ImGui::PopID();

    return headerState.m_shouldRemove;
}

ego::editor::EntityInspectorWindow::ComponentHeaderState ego::editor::EntityInspectorWindow::drawComponentHeader(const char* _componentName, bool _canRemove)
{
    bool isComponentVisible = true;

    ImGui::PushStyleColor(ImGuiCol_Header, entity_inspector_style::ComponentHeaderColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, entity_inspector_style::ComponentHeaderHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, entity_inspector_style::ComponentHeaderActiveColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, entity_inspector_style::ComponentHeaderPadding);

    const ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;
    ComponentHeaderState headerState;
    headerState.m_isExpanded =
        _canRemove ? ImGui::CollapsingHeader(_componentName, &isComponentVisible, headerFlags) : ImGui::CollapsingHeader(_componentName, headerFlags);
    headerState.m_shouldRemove = !isComponentVisible;

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    return headerState;
}

void ego::editor::EntityInspectorWindow::drawComponentProperties(ecs::Component& _component)
{
    const rtti::TypeMetaInfo& typeMetaInfo = _component.getObjectTypeMetaInfo();
    const rtti::TypeMetaInfo::PropertyRange properties = typeMetaInfo.getProperties();

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, entity_inspector_style::PropertyCellPadding);
    if (properties.empty())
    {
        ImGui::Indent(entity_inspector_style::EmptyPropertyIndent);
        ImGui::TextDisabled("No editable properties");
        ImGui::Unindent(entity_inspector_style::EmptyPropertyIndent);
    }
    else if (ImGui::BeginTable("Properties", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_NoSavedSettings))
    {
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch, entity_inspector_style::PropertyNameColumnWeight);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, entity_inspector_style::PropertyValueColumnWeight);

        gui::DrawProperties(&_component, typeMetaInfo);
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
    ImGui::Spacing();
}

void ego::editor::EntityInspectorWindow::drawAddComponentButton()
{
    ImGui::Spacing();
    ImGui::BeginDisabled();
    ImGui::Button("Add Component", ImVec2(-FLT_MIN, 0.0f));
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("Component creation is not available yet");
    }
}

bool ego::editor::EntityInspectorWindow::IsComponentVisible(rtti::TypeMetaInfoID _componentTypeMetaInfoID)
{
    return _componentTypeMetaInfoID != NameComponent::GetMetaInfoID() && _componentTypeMetaInfoID != Level::GetHierarchyComponentTypeMetaInfoID();
}

bool ego::editor::EntityInspectorWindow::IsComponentRemovable(rtti::TypeMetaInfoID _componentTypeMetaInfoID)
{
    return _componentTypeMetaInfoID != TransformComponent::GetMetaInfoID();
}
