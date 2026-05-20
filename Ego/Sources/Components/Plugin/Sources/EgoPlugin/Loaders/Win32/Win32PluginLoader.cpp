#if defined(WIN32) || defined(_WIN32)

#include "Win32PluginLoader.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/FileName/FileNameUtils.h"
#include "EgoCore/String/Format.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/ExternalPluginCore.h"
#include "EgoPlugin/ExternalModuleCore.h"

#include <Windows.h>
#include <commdlg.h>

void OutputError(const ego::FileName& _name)
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0,
        NULL
    );

#ifdef _MSC_VER
    OutputDebugStringA(
        ego::StringFormat(
            "Plugin issue (\"{}\") error message:\n {}\n",
            _name.c_str(),
            reinterpret_cast<LPTSTR>(&lpMsgBuf)
        ).c_str()
    );
#endif // _MSC_VER

    if (lpMsgBuf)
    {
        LocalFree(lpMsgBuf);
    }
}

ego::FileName ego::Win32PluginLoader::selectPluginModule(const char* _typeName)
{
    char moduleName[MAX_PATH] = {};

    const std::string title = _typeName
        ? StringFormat("Select {} plugin module", _typeName)
        : "Select plugin module";

    OPENFILENAMEA openFileName = {};
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = GetActiveWindow();
    openFileName.lpstrFile = moduleName;
    openFileName.nMaxFile = MAX_PATH;
    openFileName.lpstrFilter = "Dynamic Libraries (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
    openFileName.nFilterIndex = 1;
    openFileName.lpstrTitle = title.c_str();
    openFileName.lpstrDefExt = "dll";
    openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!GetOpenFileNameA(&openFileName))
    {
        const DWORD error = CommDlgExtendedError();
        if (error != 0)
        {
            OutputDebugStringA(StringFormat("Plugin select dialog error: {}\n", error).c_str());
        }

        return FileName();
    }

    return FileName(moduleName);
}

ego::PluginPointer ego::Win32PluginLoader::loadPlugin(const PluginModulePointer& _module, const char* _typeName)
{
    const std::string functionName = StringFormat("{}{}", EGO_TO_STRING_DEF(EGO_PLUGIN_CREATE_FUNC_BASE), _typeName);
    const PluginCreatingFunctionPtr creationFunction = reinterpret_cast<PluginCreatingFunctionPtr>(
        GetProcAddress(reinterpret_cast<HMODULE>(_module->getInfo().m_handle), functionName.c_str())
    );

    if (!creationFunction)
    {
        return nullptr;
    }

    return PluginPointer(creationFunction(_module), PluginDeleter{});
}

void* ego::Win32PluginLoader::loadNativeModule(const FileName& _moduleName)
{
    FileName dirPath;
    file_name_utils::GetFileDirPath(_moduleName, dirPath);

    SetDllDirectory(dirPath.c_str());

    void* moduleHandle = LoadLibrary(_moduleName.c_str());
    if (moduleHandle == nullptr)
    {
        OutputError(_moduleName);
        return nullptr;
    }

    return moduleHandle;
}

void ego::Win32PluginLoader::unloadNativeModule(void* _moduleHandle, const FileName& _moduleName)
{
    EGO_ASSERT(_moduleHandle != nullptr);
    if (!FreeLibrary(reinterpret_cast<HMODULE>(_moduleHandle)))
    {
        OutputError(_moduleName);
    }
}

void ego::Win32PluginLoader::initModule(
    void* _moduleHandle,
    const PluginModuleInfo& _info,
    PluginModuleBindingBridge& _bindings
)
{
    const ModuleInitFunctionPtr initFunction = reinterpret_cast<ModuleInitFunctionPtr>(
        GetProcAddress(reinterpret_cast<HMODULE>(_moduleHandle), EGO_TO_STRING_DEF(EGO_MODULE_INIT_FUNC))
    );

    if (!initFunction)
    {
        return;
    }

    return initFunction(_info, _bindings);
}

void ego::Win32PluginLoader::releaseModule(void* _moduleHandle)
{
    const ModuleReleaseFunctionPtr releaseFunction = reinterpret_cast<ModuleReleaseFunctionPtr>(
        GetProcAddress(
            reinterpret_cast<HMODULE>(_moduleHandle),
            EGO_TO_STRING_DEF(EGO_MODULE_RELEASE_FUNC)
        )
    );

    if (!releaseFunction)
    {
        return;
    }

    return releaseFunction();
}
#endif
