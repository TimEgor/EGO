#pragma once

#include <string_view>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/RTTI/Type/TypeMetaInfoID.h"

#include "EgoEditor/Gui/Window/GuiWindow.h"

namespace ego
{
    class Level;
}

namespace ego::ecs
{
    class Entity;
    struct Component;
} // namespace ego::ecs

namespace ego::editor
{
    class EditorController;

    class EntityInspectorWindow final : public GuiWindow, public NonCopyable
    {
    public:
        EntityInspectorWindow() = default;

    private:
        struct ComponentHeaderState final
        {
            bool m_isExpanded = false;
            bool m_shouldRemove = false;
        };

        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
        void drawContent();
        void drawSelectedEntity(EditorController& _editorController);
        void drawEntityHeader(Level& _level, ecs::Entity _entity);
        void drawEntityName(Level& _level, ecs::Entity _entity);
        void drawEntityMetadata(const Level& _level, ecs::Entity _entity);
        void drawComponents(Level& _level, ecs::Entity _entity);
        bool drawComponent(ecs::Component& _component);
        ComponentHeaderState drawComponentHeader(const char* _componentName, bool _canRemove);
        void drawComponentProperties(ecs::Component& _component);
        void drawAddComponentButton();
        static bool IsComponentVisible(rtti::TypeMetaInfoID _componentTypeMetaInfoID);
        static bool IsComponentRemovable(rtti::TypeMetaInfoID _componentTypeMetaInfoID);
    };

    EGO_POINTER(EntityInspectorWindow);
} // namespace ego::editor
