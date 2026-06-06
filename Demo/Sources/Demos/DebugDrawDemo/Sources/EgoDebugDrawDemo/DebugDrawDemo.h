#pragma once

#include "EgoECS/Entity.h"

#include "EgoEngine/Level/Level.h"

#include "EgoFramework/GameLogic.h"

namespace ego::demo
{
    class DebugDrawDemo final : public framework::GameLogic
    {
    public:
        DebugDrawDemo() = default;

        bool init() override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        void drawStaticPrimitives();
        void drawAnimatedPrimitives();

        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
        float m_time = 0.0f;
    };

    EGO_POINTER(DebugDrawDemo);
}
