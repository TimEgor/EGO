#pragma once

#include "EgoGui/GuiController.h"

#include "EgoEngine/Level/Level.h"
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
        bool createGuiTree(const engine::EngineSessionPointer& _engineSession);
        bool createLevel(const engine::EngineSessionPointer& _engineSession);

        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
