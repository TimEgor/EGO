#pragma once

#include "GuiMenuLayer.h"

namespace ego::editor
{
    class WindowMenuLayer final : public GuiMenuLayer
    {
    private:
        float drawMenu() override;
    };

    EGO_POINTER(WindowMenuLayer);
} // namespace ego::editor
