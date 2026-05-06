#pragma once

#include "EgoCore/PlatformMacros.h"

#include "EgoEngine/Platform/Platform.h"

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

        HINSTANCE getInstanceHandle() const;

    private:
        bool initWindowClass();

        Win32MainWindowProvider* m_mainWindowProvider;
        Win32WindowEventController* m_platformEventController;

        HINSTANCE m_instance;
    };
}
