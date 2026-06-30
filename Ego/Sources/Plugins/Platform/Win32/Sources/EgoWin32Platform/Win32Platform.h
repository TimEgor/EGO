#pragma once

#include "EgoCore/PlatformMacros.h"

#include "EgoEngine/Platform/Platform.h"

#include "FileSystem/Win32FileSystem.h"

namespace ego::win32
{
    class Win32Platform final : public Platform
    {
    public:
        Win32Platform(HINSTANCE _instance);

        bool init() override;
        void release() override;

        FileSystemPointer getFileSystem() override;

        HINSTANCE getInstanceHandle() const;

    private:
        Win32FileSystemPointer m_fileSystem;

        HINSTANCE m_instance;
        bool m_isInitialized = false;
    };
} // namespace ego::win32
