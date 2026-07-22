#pragma once

#include <string>
#include <vector>

#include "EgoECS/Entity.h"

#include "EgoGui/Gui.h"

#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/EngineLogic.h"

namespace ego::demo
{
    class GuiDemo final : public engine::EngineLogic
    {
    public:
        bool init(const InitData& _initData) override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        gui::WindowPointer createWindow() const;
        gui::WindowPointer createToolWindow(const std::string& _title, const gui::Rect& _bounds, const std::string& _description) const;

        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        gui::ViewportWeakPointer m_viewport;
        std::vector<gui::WindowPointer> m_windows;
        bool m_wasDockingEnabled = false;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
