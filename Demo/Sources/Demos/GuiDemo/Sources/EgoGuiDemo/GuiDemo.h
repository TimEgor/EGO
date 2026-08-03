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
        ~GuiDemo() override;

        bool init(const engine::EngineSessionWeakPointer& _engineSession) override;
        void update(float _deltaTime) override;

    private:
        void release();

        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        GuiLayer m_guiLayer;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
