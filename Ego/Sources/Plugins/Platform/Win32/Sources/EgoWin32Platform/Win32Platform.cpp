#include "Win32Platform.h"

#include "EgoCore/UtilsMacros.h"

ego::win32::Win32Platform::Win32Platform(HINSTANCE _instance)
    : m_instance(_instance)
{
}

bool ego::win32::Win32Platform::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    m_fileSystem = new Win32FileSystem();
    EGO_CHECK_INITIALIZATION(m_fileSystem && m_fileSystem->init());

    m_isInitialized = true;

    return true;
}

void ego::win32::Win32Platform::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_fileSystem);
    m_isInitialized = false;
}

ego::FileSystemPointer ego::win32::Win32Platform::getFileSystem()
{
    return m_fileSystem;
}

HINSTANCE ego::win32::Win32Platform::getInstanceHandle() const
{
    return m_instance;
}
