#pragma once

#include "EgoGui/Gui.h"

#include "EgoEngine/Project/EngineLogic.h"

namespace ego::demo
{
    class GuiDemo final : public engine::EngineLogic
    {
    public:
        bool init(const InitData& _initData) override;
        void release() override;

    private:
        gui::WindowPointer createWindow() const;

        gui::ViewportWeakPointer m_viewport;
        gui::WindowPointer m_window = nullptr;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
