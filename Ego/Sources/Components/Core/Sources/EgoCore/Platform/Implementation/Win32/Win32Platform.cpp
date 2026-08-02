#include "Win32Platform.h"

#include <array>
#include <bit>
#include <cstddef>
#include <string>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/StringConverter.h"
#include "EgoCore/UtilsMacros.h"
#include "Input/Win32InputDeviceProvider.h"
#include "Surface/Win32PlatformSurfaceController.h"

#include <shobjidl.h>

namespace
{
    template <std::size_t Size>
    constexpr GUID MakeDialogClientGuid(const char (&_identifier)[Size])
    {
        static_assert(Size > 1);
        static_assert(Size <= sizeof(GUID) + 1);

        std::array<std::byte, sizeof(GUID)> clientGuidData = {};
        for (std::size_t characterIndex = 0; characterIndex < Size - 1; ++characterIndex)
        {
            clientGuidData[characterIndex] = static_cast<std::byte>(_identifier[characterIndex]);
        }

        return std::bit_cast<GUID>(clientGuidData);
    }

    constexpr GUID SelectFileDialogClientGuid = MakeDialogClientGuid("Ego.SelectFile");
    constexpr GUID SelectFolderDialogClientGuid = MakeDialogClientGuid("Ego.SelectFolder");
}

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

    m_fileSystem = MakePointer<Win32FileSystem>();
    EGO_CHECK_INITIALIZATION(m_fileSystem && m_fileSystem->init());

    m_inputDeviceController = MakePointer<InputDeviceController>();
    EGO_CHECK_INITIALIZATION(
        m_inputDeviceController &&
        m_inputDeviceController->addProvider(MakePointer<Win32InputDeviceProvider>()) &&
        m_inputDeviceController->init());

    m_surfaceController = MakePointer<Win32PlatformSurfaceController>(m_instance);
    EGO_CHECK_INITIALIZATION(m_surfaceController && m_surfaceController->init());

    m_isInitialized = true;

    return true;
}

void ego::win32::Win32Platform::release()
{
    if (!m_isInitialized && !m_surfaceController && !m_inputDeviceController && !m_fileSystem)
    {
        return;
    }

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_surfaceController);
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

ego::PlatformSurfaceController& ego::win32::Win32Platform::getSurfaceController()
{
    EGO_ASSERT(m_surfaceController);
    return *m_surfaceController;
}

ego::FileName ego::win32::Win32Platform::selectOpenFile(const Platform::SelectFileDialogParams& _params) const
{
    const HRESULT initializationResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(initializationResult) && initializationResult != RPC_E_CHANGED_MODE)
    {
        return FileName();
    }

    FileName selectedFile;
    IFileOpenDialog* dialog = nullptr;
    const HRESULT creationResult = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(creationResult))
    {
        FILEOPENDIALOGOPTIONS options = 0;
        if (SUCCEEDED(dialog->SetClientGuid(SelectFileDialogClientGuid)) &&
            SUCCEEDED(dialog->GetOptions(&options)) &&
            SUCCEEDED(dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR)))
        {
            std::vector<std::wstring> filterNames;
            std::vector<std::wstring> filterPatterns;
            filterNames.reserve(_params.m_filterCount);
            filterPatterns.reserve(_params.m_filterCount);

            if (_params.m_filters)
            {
                for (std::size_t filterIndex = 0; filterIndex < _params.m_filterCount; ++filterIndex)
                {
                    const Platform::OpenFileDialogFilter& filter = _params.m_filters[filterIndex];
                    if (!filter.m_name || !filter.m_pattern)
                    {
                        continue;
                    }

                    filterNames.push_back(ConvertStringToWString(filter.m_name));
                    filterPatterns.push_back(ConvertStringToWString(filter.m_pattern));
                }
            }

            if (filterNames.empty())
            {
                filterNames.emplace_back(L"All Files (*.*)");
                filterPatterns.emplace_back(L"*.*");
            }

            std::vector<COMDLG_FILTERSPEC> filterSpecs;
            filterSpecs.reserve(filterNames.size());
            for (std::size_t filterIndex = 0; filterIndex < filterNames.size(); ++filterIndex)
            {
                filterSpecs.push_back({filterNames[filterIndex].c_str(), filterPatterns[filterIndex].c_str()});
            }

            dialog->SetFileTypes(static_cast<UINT>(filterSpecs.size()), filterSpecs.data());
            dialog->SetFileTypeIndex(1);

            if (_params.m_title)
            {
                const std::wstring title = ConvertStringToWString(_params.m_title);
                dialog->SetTitle(title.c_str());
            }

            if (_params.m_defaultExtension)
            {
                const std::wstring defaultExtension = ConvertStringToWString(_params.m_defaultExtension);
                dialog->SetDefaultExtension(defaultExtension.c_str());
            }

            const HWND ownerWindow = _params.m_ownerWindowHandle ? static_cast<HWND>(_params.m_ownerWindowHandle) : GetActiveWindow();
            if (SUCCEEDED(dialog->Show(ownerWindow)))
            {
                IShellItem* selectedItem = nullptr;
                if (SUCCEEDED(dialog->GetResult(&selectedItem)))
                {
                    PWSTR selectedPath = nullptr;
                    if (SUCCEEDED(selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &selectedPath)))
                    {
                        selectedFile = ConvertWStringToString(selectedPath);
                        CoTaskMemFree(selectedPath);
                    }

                    selectedItem->Release();
                }
            }
        }

        dialog->Release();
    }

    if (SUCCEEDED(initializationResult))
    {
        CoUninitialize();
    }

    return selectedFile;
}

ego::FileName ego::win32::Win32Platform::selectDirectory(const Platform::SelectDirectoryDialogParams& _params) const
{
    const HRESULT initializationResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(initializationResult) && initializationResult != RPC_E_CHANGED_MODE)
    {
        return FileName();
    }

    FileName selectedDirectory;
    IFileOpenDialog* dialog = nullptr;
    const HRESULT creationResult = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (SUCCEEDED(creationResult))
    {
        FILEOPENDIALOGOPTIONS options = 0;
        if (SUCCEEDED(dialog->SetClientGuid(SelectFolderDialogClientGuid)) &&
            SUCCEEDED(dialog->GetOptions(&options)) &&
            SUCCEEDED(dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST)))
        {
            if (_params.m_title)
            {
                const std::wstring title = ConvertStringToWString(_params.m_title);
                dialog->SetTitle(title.c_str());
            }

            const HWND ownerWindow = _params.m_ownerWindowHandle ? static_cast<HWND>(_params.m_ownerWindowHandle) : GetActiveWindow();
            if (SUCCEEDED(dialog->Show(ownerWindow)))
            {
                IShellItem* selectedItem = nullptr;
                if (SUCCEEDED(dialog->GetResult(&selectedItem)))
                {
                    PWSTR selectedPath = nullptr;
                    if (SUCCEEDED(selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &selectedPath)))
                    {
                        selectedDirectory = ConvertWStringToString(selectedPath);
                        CoTaskMemFree(selectedPath);
                    }

                    selectedItem->Release();
                }
            }
        }

        dialog->Release();
    }

    if (SUCCEEDED(initializationResult))
    {
        CoUninitialize();
    }

    return selectedDirectory;
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
