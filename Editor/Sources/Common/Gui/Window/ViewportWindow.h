#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "GuiWindow.h"

namespace ego::editor
{
    class ViewportWindow final
        : public GuiWindow, public NonCopyable
    {
    public:
        ViewportWindow() = default;

    private:
        std::string_view getTitle() const override;
        void drawWindow(bool& _isVisible) override;
    };

    EGO_POINTER(ViewportWindow);
} // namespace ego::editor
