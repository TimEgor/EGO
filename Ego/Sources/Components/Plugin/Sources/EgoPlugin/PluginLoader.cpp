#include "PluginLoader.h"

ego::PluginModulePointer ego::PluginLoader::loadModule(
    PluginModuleID _moduleID,
    const FileName& _moduleName,
    PluginModuleBindingBridge& _bindings
)
{
    void* moduleHandle = loadNativeModule(_moduleName);
    if (!moduleHandle)
    {
        return nullptr;
    }

    PluginModuleInfo moduleInfo;
    moduleInfo.m_handle = moduleHandle;
    moduleInfo.m_moduleId = _moduleID;
    moduleInfo.m_modulePath = _moduleName;

    initModule(moduleHandle, moduleInfo, _bindings);

    return PluginModulePointer(new PluginModule(moduleInfo));
}

void ego::PluginLoader::unloadModule(void* _moduleHandle, const FileName& _moduleName)
{
    releaseModule(_moduleHandle);
    unloadNativeModule(_moduleHandle, _moduleName);
}
