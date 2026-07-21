#pragma once

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

        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        gui::ViewportWeakPointer m_viewport;
        gui::WindowPointer m_window = nullptr;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
