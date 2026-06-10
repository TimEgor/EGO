#pragma once

#include "EgoEngine/Graphic/Render/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/Render/Resources/MeshResource.h"
#include "EgoEngine/Level/Level.h"

#include "EgoFramework/GameLogic.h"

namespace ego::demo
{
    class TestDemo final : public framework::GameLogic
    {
    public:
        TestDemo() = default;

        bool init() override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        LevelPointer m_level = nullptr;
        ecs::Entity m_meshEntity;
        render::MeshResourcePointer m_triangleMesh = nullptr;
        render::MaterialResourcePointer m_triangleMaterial = nullptr;
        float m_meshRotationAngle = 0.0f;
    };

    EGO_POINTER(TestDemo);
}
