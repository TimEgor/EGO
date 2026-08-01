#pragma once

#include "GuiMenuLayer.h"

namespace ego::editor
{
    class EditorProjectController;

    class ProjectMenuLayer final : public GuiMenuLayer
    {
    public:
        explicit ProjectMenuLayer(EditorProjectController& _projectController);

    private:
        float drawMenu() override;

        EditorProjectController& m_projectController;
    };

    EGO_POINTER(ProjectMenuLayer);
} // namespace ego::editor
