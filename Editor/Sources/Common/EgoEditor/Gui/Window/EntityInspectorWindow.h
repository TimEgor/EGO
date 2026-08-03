#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEditor/Gui/Window/GuiWindow.h"

namespace ego::editor
{
    class EntityInspectorWindow final : public GuiWindow, public NonCopyable
    {
    public:
        EntityInspectorWindow() = default;

    private:
        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
    };

    EGO_POINTER(EntityInspectorWindow);
} // namespace ego::editor
