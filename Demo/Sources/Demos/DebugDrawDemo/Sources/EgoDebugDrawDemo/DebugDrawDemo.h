#pragma once

#include "EgoECS/Entity.h"

#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/EngineLogic.h"

namespace ego::demo
{
    class DebugDrawDemo final : public engine::EngineLogic
    {
    public:
        DebugDrawDemo() = default;
        ~DebugDrawDemo() override;

        bool init(const engine::EngineSessionWeakPointer& _engineSession) override;
        void update(float _deltaTime) override;

    private:
        void release();

        void drawStaticPrimitives(const engine::EngineSessionPointer& _engineSession);
        void drawAnimatedPrimitives(const engine::EngineSessionPointer& _engineSession);

        engine::EngineSessionWeakPointer m_engineSession;
        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        float m_time = 0.0f;
    };

    EGO_POINTER(DebugDrawDemo);
} // namespace ego::demo
