#pragma once

#include "EgoCore/Pointer/Pointer.h"

namespace ego::editor
{
    class GuiMenuLayer
    {
    public:
        virtual ~GuiMenuLayer() = default;

        virtual float drawMenu() = 0;

    protected:
        GuiMenuLayer() = default;
    };

    EGO_POINTER(GuiMenuLayer);
} // namespace ego::editor
