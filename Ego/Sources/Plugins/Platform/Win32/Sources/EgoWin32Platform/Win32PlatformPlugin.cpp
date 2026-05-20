#include "Win32PlatformPlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "Win32Platform.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::win32::Win32PlatformPlugin, PlatformPlugin, ego::PlatformPlugin);

ego::win32::Win32PlatformPlugin::Win32PlatformPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : PlatformPlugin(_module, _pluginType)
{
    
}

ego::PlatformPointer ego::win32::Win32PlatformPlugin::createPlatform(void* _platformNativeInstance)
{
    return PlatformPointer(new Win32Platform(static_cast<HINSTANCE>(_platformNativeInstance)));
}
