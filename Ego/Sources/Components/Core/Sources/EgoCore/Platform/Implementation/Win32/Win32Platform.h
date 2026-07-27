#pragma once

#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformMacros.h"
#include "FileSystem/Win32FileSystem.h"

namespace ego::win32
{
    class Win32PlatformSurfaceController;

    class Win32Platform final
        : public Platform
    {
    public:
        Win32Platform(HINSTANCE _instance);

        bool init() override;
        void release() override;

        FileSystemPointer getFileSystem() override;
        InputDeviceController& getInputDeviceController() override;
        PlatformSurfaceController& getSurfaceController() override;
        FileName selectOpenFile(const Platform::OpenFileDialogParams& _params) const override;

        Platform::DynamicLibraryHandle loadDynamicLibrary(const FileName& _libraryPath) override;
        void unloadDynamicLibrary(Platform::DynamicLibraryHandle _libraryHandle, const FileName& _libraryPath) override;
        void* getDynamicLibrarySymbol(Platform::DynamicLibraryHandle _libraryHandle, const char* _symbolName) override;

        HINSTANCE getInstanceHandle() const;

    private:
        static void OutputDynamicLibraryError(const FileName& _libraryPath);

        Win32FileSystemPointer m_fileSystem;
        InputDeviceControllerPointer m_inputDeviceController;
        SharedPointer<Win32PlatformSurfaceController> m_surfaceController;

        HINSTANCE m_instance;
        bool m_isInitialized = false;
    };
} // namespace ego::win32
