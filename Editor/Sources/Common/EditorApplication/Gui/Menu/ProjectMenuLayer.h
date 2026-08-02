#pragma once

#include "GuiMenuLayer.h"

namespace ego::editor
{
    class ProjectMenuLayer final : public GuiMenuLayer
    {
    private:
        float drawMenu() override;
    };

    EGO_POINTER(ProjectMenuLayer);
} // namespace ego::editor
