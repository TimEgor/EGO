#include "Win32MainWindowProvider.h"

#include "EgoEngine/Engine.h"

bool ego::win32::Win32MainWindowProvider::prepareMainWindow(const char* _title, const WindowSize& _size)
{
    m_mainWindow = engine::GetEngine().getPlatform().createWindow(_title, _size);
    EGO_CHECK_RETURN_FALSE(m_mainWindow);

    return true;
}

bool ego::win32::Win32MainWindowProvider::isWindowPlatformProvided() const
{
    return false;
}

ego::WindowPointer ego::win32::Win32MainWindowProvider::getMainWindow() const
{
    return m_mainWindow;
}
