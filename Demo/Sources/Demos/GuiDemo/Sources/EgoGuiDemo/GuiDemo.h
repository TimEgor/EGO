#pragma once

#include <string>

#include "EgoGui/Gui.h"

#include "EgoEngine/Project/EngineLogic.h"

namespace ego::demo
{
    class GuiDemo final : public engine::EngineLogic
    {
    public:
        GuiDemo() = default;

        bool init(const InitData& _initData) override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        gui::GuiWindowPointer createGuiWindow(const std::string& _title, const gui::GuiPosition& _position, const gui::GuiSize& _size);

        engine::EngineSessionWeakPointer m_engineSession;
        gui::GuiWindowPointer m_sceneSettingsWindow = nullptr;
        gui::GuiWindowPointer m_renderSettingsWindow = nullptr;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
