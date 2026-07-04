#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"

namespace ego::render
{
    class DefaultRenderFileSystems final
    {
    public:
        DefaultRenderFileSystems() = default;
        ~DefaultRenderFileSystems();

        bool loadAssetsRootPath(FileName& _assetsRootPath);
        bool initAssetsFileSystem(const FileName& _assetsRootPath);
        void release();

    private:
        static RootedFileSystemPointer CreateFileSystem(const FileName& _rootPath);

        RootedFileSystemPointer m_assetsFileSystem = nullptr;
    };
} // namespace ego::render
