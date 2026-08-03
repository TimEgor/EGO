#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEditor/Gui/Window/GuiWindow.h"

namespace ego::editor
{
    class SceneInspectorWindow final : public GuiWindow, public NonCopyable
    {
    public:
        SceneInspectorWindow() = default;

    private:
        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
    };

    EGO_POINTER(SceneInspectorWindow);
} // namespace ego::editor
