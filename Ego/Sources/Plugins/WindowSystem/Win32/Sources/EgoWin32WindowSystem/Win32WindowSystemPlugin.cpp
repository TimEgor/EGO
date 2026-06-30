#include "Win32WindowSystemPlugin.h"

#include "EgoApplication/Plugin/ApplicationExternalModuleCore.h"
#include "EgoPlugin/ExternalPluginCore.h"

#include "Win32WindowSystem.h"

EGO_CORE_MODULE();
EGO_APPLICATION_MODULE();

EGO_PLUGIN_CREATE(ego::win32::Win32WindowSystemPlugin, WindowSystemPlugin, ego::WindowSystemPlugin);

ego::win32::Win32WindowSystemPlugin::Win32WindowSystemPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : WindowSystemPlugin(_module, _pluginType)
{
}

ego::WindowSystemPointer ego::win32::Win32WindowSystemPlugin::createWindowSystem(void* _nativeInstanceHandle)
{
    return WindowSystemPointer(new Win32WindowSystem(static_cast<HINSTANCE>(_nativeInstanceHandle)));
}
