#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEditor/Gui/Window/GuiWindow.h"

namespace ego::ecs
{
    struct Component;
}

namespace ego::gui
{
    class PropertyInspector;
}

namespace ego::rtti
{
    class PropertyMetaInfo;
    class TypeMetaInfo;
} // namespace ego::rtti

namespace ego::editor
{
    class EntityInspectorWindow final : public GuiWindow, public NonCopyable
    {
    public:
        EntityInspectorWindow() = default;

    private:
        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
        void drawComponent(ecs::Component& _component, const gui::PropertyInspector& _propertyInspector);
        void drawProperties(void* _object, const rtti::TypeMetaInfo& _typeMetaInfo, const gui::PropertyInspector& _propertyInspector);
        void drawProperty(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const gui::PropertyInspector& _propertyInspector);
    };

    EGO_POINTER(EntityInspectorWindow);
} // namespace ego::editor
