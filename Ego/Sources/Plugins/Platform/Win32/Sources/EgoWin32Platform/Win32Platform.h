#pragma once

#include "EgoCore/PlatformMacros.h"

#include "EgoEngine/Platform/Platform.h"

#include "FileSystem/Win32FileSystem.h"
#include "Window/Win32MainWindowProvider.h"
#include "Win32PlatformEventController.h"

namespace ego::win32
{
    class Win32Platform final : public Platform
    {
    public:
        Win32Platform(HINSTANCE _instance);

        virtual bool init() override;
        virtual void release() override;

        virtual MainWindowProvider& getMainWindowProvider() override;
        virtual const MainWindowProvider& getMainWindowProvider() const override;
        virtual WindowPointer createWindow(const char* _title, const WindowSize& _size) override;

        virtual const PlatformEventController& getPlatformEventController() const override;
        virtual PlatformEventController& getPlatformEventController() override;

        virtual FileSystemPointer getFileSystem() override;

        HINSTANCE getInstanceHandle() const;

    private:
        bool initWindowClass();

        Win32MainWindowProviderPointer m_mainWindowProvider = nullptr;
        Win32WindowEventControllerPointer m_platformEventController = nullptr;
        Win32FileSystemPointer m_fileSystem;

        HINSTANCE m_instance;
        bool m_isInitialized = false;
    };
}
