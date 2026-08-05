#pragma once

#include "EgoCore/Math/Transform.h"

#include "EgoEngine/Graphic/SceneRender/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/SceneRender/Resources/MeshResource.h"
#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/EngineLogic.h"

namespace ego
{
    class ResourceController;
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::demo
{
    class TestDemo final : public engine::EngineLogic
    {
    public:
        TestDemo() = default;
        ~TestDemo() override;

        bool init(const engine::EngineSessionWeakPointer& _engineSession) override;
        void update(float _deltaTime) override;

    private:
        void release();

        bool createTriangleEntity(ecs::Entity& _entity, const render::MaterialResourcePointer& _materialResource, const TransformVector& _position);
        bool setTriangleTransform(ecs::Entity _entity, const TransformVector& _position, float _rotationAngle);

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
