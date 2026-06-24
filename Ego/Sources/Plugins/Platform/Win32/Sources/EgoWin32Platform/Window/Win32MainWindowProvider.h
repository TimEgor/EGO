#pragma once

#include "EgoEngine/Platform/Window/MainWindowProvider.h"

namespace ego::win32
{
    class Win32MainWindowProvider final : public MainWindowProvider
    {
    public:
        Win32MainWindowProvider() = default;

        bool prepareMainWindow(const char* _title, const WindowSize& _size) override;

        bool isWindowPlatformProvided() const override;
        WindowPointer getMainWindow() const override;

        EGO_RTTI_VIRTUAL(Win32MainWindowProvider, MainWindowProvider);

    private:
        WindowPointer m_mainWindow;
    };

    EGO_POINTER(Win32MainWindowProvider);
} // namespace ego::win32
