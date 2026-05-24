#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoEngine/Graphic/Render/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/Render/Resources/MeshResource.h"
#include "EgoEngine/Platform/FileSystem/RootedFileSystem.h"
#include "EgoEngine/Resources/GeneralResources/TextResource.h"
#include "EgoFramework/GameLogic.h"

#include <string>
#include <vector>

namespace ego::demo
{
    class TestDemo final : public framework::GameLogic
    {
    public:
        TestDemo() = default;
        explicit TestDemo(const FileName& _projectPath);

        bool init() override;
        void release() override;

        const std::string& getLoadedText() const;

    private:
        FileName m_projectPath;
        std::vector<RootedFileSystemPointer> m_assetFileSystems;
        TextResourcePointer m_textResource = nullptr;
        MeshResourcePointer m_triangleMesh = nullptr;
        MaterialResourcePointer m_triangleMaterial = nullptr;
        std::string m_loadedText;
    };

    EGO_POINTER(TestDemo);
}
