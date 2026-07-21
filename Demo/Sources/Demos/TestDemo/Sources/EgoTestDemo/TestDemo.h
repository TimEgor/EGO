#pragma once

#include "EgoCore/Math/ComputeVector3.h"

#include "EgoEngine/Graphic/SceneRender/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/SceneRender/Resources/MeshResource.h"
#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/EngineLogic.h"

namespace ego::demo
{
    class TestDemo final : public engine::EngineLogic
    {
    public:
        TestDemo() = default;

        bool init(const InitData& _initData) override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        bool createTriangleEntity(ecs::Entity& _entity, const render::MaterialResourcePointer& _materialResource, const ComputeVector3& _position);
        bool setTriangleTransform(ecs::Entity _entity, const ComputeVector3& _position, float _rotationAngle);

        engine::EngineSessionWeakPointer m_engineSession;
        ResourceControllerPointer m_resourceController = nullptr;
        LevelPointer m_level = nullptr;
        ecs::Entity m_firstTriangleEntity;
        ecs::Entity m_secondTriangleEntity;
        render::MeshResourcePointer m_triangleMesh = nullptr;
        render::MaterialResourcePointer m_firstTriangleMaterial = nullptr;
        render::MaterialResourcePointer m_secondTriangleMaterial = nullptr;
        float m_triangleRotationAngle = 0.0f;
    };

    EGO_POINTER(TestDemo);
} // namespace ego::demo
