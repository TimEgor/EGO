#include "PluginLoader.h"

#include <string>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/String/Format.h"
#include "EgoCore/UtilsMacros.h"

#include "ExternalModule.h"
#include "ExternalPlugin.h"
#include "PluginCatalog.h"
#include "PluginSubsystem.h"

namespace
{
    ego::FileName SelectPluginModuleFromCatalog(const char* _typeName)
    {
        if (!_typeName || _typeName[0] == '\0')
        {
            return ego::FileName();
        }

        const ego::PluginSubsystemPointer pluginSubsystem = ego::GetPluginSubsystemPointer();
        if (!pluginSubsystem)
        {
            return ego::FileName();
        }

        const ego::PluginCatalogPointer pluginCatalog = pluginSubsystem->getPluginCatalogPointer();
        if (!pluginCatalog)
        {
            return ego::FileName();
        }

        return pluginCatalog->getModulePath(ego::GetPluginType(_typeName));
    }
} // namespace

ego::FileName ego::PluginLoader::selectPluginModule(const char* _typeName)
{
    const FileName catalogModuleName = SelectPluginModuleFromCatalog(_typeName);
    if (catalogModuleName)
    {
        return catalogModuleName;
    }

    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_VALUE(platform, FileName());

    const std::string title = _typeName ? StringFormat("Select {} plugin module", _typeName) : "Select plugin module";

    const Platform::OpenFileDialogFilter filters[] = {{"Dynamic Libraries (*.dll)", "*.dll"}, {"All Files (*.*)", "*.*"}};

    Platform::OpenFileDialogParams params;
    params.m_title = title.c_str();
    params.m_defaultExtension = "dll";
    params.m_filters = filters;
    params.m_filterCount = sizeof(filters) / sizeof(filters[0]);

    return platform->selectOpenFile(params);
}

ego::PluginModulePointer ego::PluginLoader::loadModule(PluginModuleID _moduleID, const FileName& _moduleName, const PluginControllerWeakPointer& _pluginController)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_NULL(platform);

    void* moduleHandle = platform->loadDynamicLibrary(_moduleName);
    if (!moduleHandle)
    {
        return nullptr;
    }

    PluginModuleInfo moduleInfo;
    moduleInfo.m_handle = moduleHandle;
    moduleInfo.m_moduleId = _moduleID;
    moduleInfo.m_modulePath = _moduleName;

    initModule(moduleHandle, moduleInfo);

    return PluginModulePointer(new PluginModule(moduleInfo, _pluginController), PluginModuleDeleter{});
}

void ego::PluginLoader::unloadModule(void* _moduleHandle, const FileName& _moduleName)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN(platform);

    releaseModule(_moduleHandle);
    platform->unloadDynamicLibrary(_moduleHandle, _moduleName);
}

ego::PluginPointer ego::PluginLoader::loadPlugin(const PluginModulePointer& _module, const char* _typeName)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN_NULL(platform);

    const std::string functionName = StringFormat("{}{}", EGO_TO_STRING_DEF(EGO_PLUGIN_CREATE_FUNC_BASE), _typeName);
    void* symbol = platform->getDynamicLibrarySymbol(_module->getInfo().m_handle, functionName.c_str());
    const auto creationFunction = reinterpret_cast<PluginCreatingFunctionPtr>(symbol);

    if (!creationFunction)
    {
        return nullptr;
    }

    return PluginPointer(creationFunction(_module), PluginDeleter{});
}

void ego::PluginLoader::initModule(void* _moduleHandle, const PluginModuleInfo& _moduleInfo)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN(platform);

    const subsystem::SubsystemRegistryPointer subsystemRegistry = subsystem::SubsystemLocator::GetInstance().getRegistryPointer();
    EGO_CHECK_RETURN(subsystemRegistry);

    void* symbol = platform->getDynamicLibrarySymbol(_moduleHandle, EGO_TO_STRING_DEF(EGO_MODULE_INIT_FUNC));
    const auto initFunction = reinterpret_cast<ModuleInitFunctionPtr>(symbol);

    if (!initFunction)
    {
        return;
    }

    return initFunction(_moduleInfo, subsystemRegistry);
}

void ego::PluginLoader::releaseModule(void* _moduleHandle)
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_CHECK_RETURN(platform);

    void* symbol = platform->getDynamicLibrarySymbol(_moduleHandle, EGO_TO_STRING_DEF(EGO_MODULE_RELEASE_FUNC));
    const auto releaseFunction = reinterpret_cast<ModuleReleaseFunctionPtr>(symbol);

    if (!releaseFunction)
    {
        return;
    }

    return releaseFunction();
}
