#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoECS/Entity.h"

#include "EgoEditor/Gui/Window/GuiWindow.h"

namespace ego
{
    class Level;
} // namespace ego

namespace ego::editor
{
    class ProjectController;

    class SceneInspectorWindow final : public GuiWindow, public NonCopyable
    {
    public:
        SceneInspectorWindow() = default;

    private:
        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
        void drawNode(ProjectController& _projectController, const Level& _level, ecs::Entity _node, ecs::Entity _selectedEntity);
    };

    EGO_POINTER(SceneInspectorWindow);
} // namespace ego::editor
