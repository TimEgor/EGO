#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEditor/Gui/Menu/GuiMenuLayer.h"

namespace ego::editor
{
    class GuiMenuController final : public NonCopyable
    {
    public:
        GuiMenuController() = default;

        bool init();
        void release();

        float draw();

    private:
        GuiMenuLayerPointer m_projectMenuLayer = nullptr;
        GuiMenuLayerPointer m_windowMenuLayer = nullptr;
    };
} // namespace ego::editor
