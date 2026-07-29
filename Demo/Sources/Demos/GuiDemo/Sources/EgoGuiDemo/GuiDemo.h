#pragma once

#include "EgoECS/Entity.h"

#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/EngineLogic.h"

#include "GuiLayer.h"

namespace ego::demo
{
    class GuiDemo final : public engine::EngineLogic
    {
    public:
        bool init(const engine::EngineSessionWeakPointer& _engineSession) override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        GuiLayer m_guiLayer;
        gui::GuiLayerID m_guiLayerID = gui::InvalidGuiLayerID;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
