#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Graphic/Render/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/Render/Resources/MeshResource.h"
#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Platform/FileSystem/RootedFileSystem.h"

#include "EgoFramework/GameLogic.h"

#include <vector>

namespace ego::demo
{
    class TestDemo final : public framework::GameLogic
    {
    public:
        TestDemo() = default;
        explicit TestDemo(const FileName& _projectPath);

        bool init() override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        FileName m_projectPath;
        std::vector<RootedFileSystemPointer> m_assetFileSystems;
        LevelPointer m_level = nullptr;
        ecs::Entity m_meshEntity;
        MeshResourcePointer m_triangleMesh = nullptr;
        MaterialResourcePointer m_triangleMaterial = nullptr;;
        float m_meshRotationAngle = 0.0f;
    };

    EGO_POINTER(TestDemo);
}
