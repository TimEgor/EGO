#pragma once

#include "EgoCore/FileName/FileName.h"

#include "EgoDemoFramework/Demo.h"
#include "EgoEngine/Graphic/Render/Resources/MaterialResource.h"
#include "EgoEngine/Graphic/Render/Resources/MeshResource.h"
#include "EgoEngine/Platform/FileSystem/RootedFileSystem.h"
#include "EgoEngine/Resources/GeneralResources/TextResource.h"

#include <string>

namespace ego::demo
{
    class TestDemo final : public Demo
    {
    public:
        TestDemo() = default;
        explicit TestDemo(const FileName& _configPath);

        bool init() override;
        void release() override;

        const std::string& getLoadedText() const;

    private:
        FileName m_configPath;
        RootedFileSystemPointer m_assetsFileSystem = nullptr;
        TextResourcePointer m_textResource = nullptr;
        MeshResourcePointer m_triangleMesh = nullptr;
        MaterialResourcePointer m_triangleMaterial = nullptr;
        std::string m_loadedText;
    };

    EGO_POINTER(TestDemo);
}
