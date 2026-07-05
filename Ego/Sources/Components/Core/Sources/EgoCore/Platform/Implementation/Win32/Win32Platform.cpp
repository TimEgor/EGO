#include "Win32Platform.h"

#include <string>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/Format.h"
#include "EgoCore/String/StringConverter.h"
#include "EgoCore/UtilsMacros.h"

#include "Input/Win32InputDeviceProvider.h"
#include "WindowSystem/Win32WindowSystem.h"

#include <commdlg.h>

namespace
{
    std::string BuildWin32Filter(const ego::Platform::OpenFileDialogFilter* _filters, const std::size_t _filterCount)
    {
        std::string result;

        if (_filters)
        {
            for (std::size_t i = 0; i < _filterCount; ++i)
            {
                const ego::Platform::OpenFileDialogFilter& filter = _filters[i];
                if (!filter.m_name || !filter.m_pattern)
                {
                    continue;
                }

                result += filter.m_name;
                result.push_back('\0');
                result += filter.m_pattern;
                result.push_back('\0');
            }
        }

        if (result.empty())
        {
            result += "All Files (*.*)";
            result.push_back('\0');
            result += "*.*";
            result.push_back('\0');
        }

        result.push_back('\0');
        return result;
    }
} // namespace

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

    m_inputDeviceController = new InputDeviceController();
    EGO_CHECK_INITIALIZATION(m_inputDeviceController && m_inputDeviceController->addProvider(new Win32InputDeviceProvider()) && m_inputDeviceController->init());

    m_windowSystem = new Win32WindowSystem(m_instance);
    EGO_CHECK_INITIALIZATION(m_windowSystem && m_windowSystem->init());

    m_isInitialized = true;

    return true;
}

void ego::win32::Win32Platform::release()
{
    if (!m_isInitialized && !m_windowSystem && !m_inputDeviceController && !m_fileSystem)
    {
        return;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_windowSystem);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_inputDeviceController);
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_fileSystem);
    m_isInitialized = false;
}

ego::FileSystemPointer ego::win32::Win32Platform::getFileSystem()
{
    return m_fileSystem;
}

ego::InputDeviceController& ego::win32::Win32Platform::getInputDeviceController()
{
    EGO_ASSERT(m_inputDeviceController);
    return *m_inputDeviceController;
}

ego::WindowSystem& ego::win32::Win32Platform::getWindowSystem()
{
    EGO_ASSERT(m_windowSystem);
    return *m_windowSystem;
}

ego::FileName ego::win32::Win32Platform::selectOpenFile(const Platform::OpenFileDialogParams& _params) const
{
    char fileName[MAX_PATH] = {};
    const std::string filter = BuildWin32Filter(_params.m_filters, _params.m_filterCount);

    OPENFILENAMEA openFileName = {};
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = _params.m_ownerWindowHandle ? static_cast<HWND>(_params.m_ownerWindowHandle) : GetActiveWindow();
    openFileName.lpstrFile = fileName;
    openFileName.nMaxFile = MAX_PATH;
    openFileName.lpstrFilter = filter.c_str();
    openFileName.nFilterIndex = 1;
    openFileName.lpstrTitle = _params.m_title;
    openFileName.lpstrDefExt = _params.m_defaultExtension;
    openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameA(&openFileName))
    {
        const DWORD error = CommDlgExtendedError();
        if (error != 0)
        {
            OutputDebugStringA(StringFormat("Open file dialog error: {}\n", error).c_str());
        }

        return FileName();
    }

    return FileName(fileName);
}

ego::Platform::DynamicLibraryHandle ego::win32::Win32Platform::loadDynamicLibrary(const FileName& _libraryPath)
{
    FileName directoryPath;
    file_name_utils::GetFileDirPath(_libraryPath, directoryPath);

    SetDllDirectory(directoryPath.c_str());

    Platform::DynamicLibraryHandle libraryHandle = LoadLibrary(_libraryPath.c_str());
    if (libraryHandle == Platform::InvalidDynamicLibraryHandle)
    {
        OutputDynamicLibraryError(_libraryPath);
    }

    return libraryHandle;
}

void ego::win32::Win32Platform::unloadDynamicLibrary(Platform::DynamicLibraryHandle _libraryHandle, const FileName& _libraryPath)
{
    EGO_ASSERT(_libraryHandle != Platform::InvalidDynamicLibraryHandle);
    if (!FreeLibrary(reinterpret_cast<HMODULE>(_libraryHandle)))
    {
        OutputDynamicLibraryError(_libraryPath);
    }
}

void* ego::win32::Win32Platform::getDynamicLibrarySymbol(Platform::DynamicLibraryHandle _libraryHandle, const char* _symbolName)
{
    EGO_ASSERT(_libraryHandle != Platform::InvalidDynamicLibraryHandle);
    EGO_ASSERT(_symbolName);

    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(_libraryHandle), _symbolName));
}

HINSTANCE ego::win32::Win32Platform::getInstanceHandle() const
{
    return m_instance;
}

void ego::win32::Win32Platform::OutputDynamicLibraryError(const FileName& _libraryPath)
{
    LPWSTR message = nullptr;
    const DWORD error = GetLastError();

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&message),
        0,
        nullptr);

    const std::wstring output = L"Dynamic library issue (\"" + ConvertStringToWString(_libraryPath.c_str()) + L"\") error " + std::to_wstring(error) + L":\n " +
                                (message != nullptr ? message : L"Unknown error") + L"\n";

    OutputDebugStringW(output.c_str());

    if (message != nullptr)
    {
        LocalFree(message);
    }
}
