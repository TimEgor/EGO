#pragma once

#include "GuiMenuLayer.h"

namespace ego::editor
{
    class GuiWindowController;

    class WindowMenuLayer final : public GuiMenuLayer
    {
    public:
        explicit WindowMenuLayer(GuiWindowController& _windowController);

    private:
        float drawMenu() override;

        GuiWindowController& m_windowController;
    };

    EGO_POINTER(WindowMenuLayer);
} // namespace ego::editor
