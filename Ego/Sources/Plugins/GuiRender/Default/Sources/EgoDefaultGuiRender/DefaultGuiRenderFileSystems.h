#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Platform/FileSystem/RootedFileSystem.h"

namespace ego::gui::default_gui_render
{
    class DefaultGuiRenderFileSystems final
    {
    public:
        DefaultGuiRenderFileSystems() = default;
        ~DefaultGuiRenderFileSystems();

        bool loadAssetsRootPath(FileName& _assetsRootPath);
        bool initAssetsFileSystem(const FileName& _assetsRootPath);
        void release();

    private:
        static RootedFileSystemPointer CreateFileSystem(const FileName& _rootPath);

        RootedFileSystemPointer m_assetsFileSystem = nullptr;
    };
} // namespace ego::gui::default_gui_render
