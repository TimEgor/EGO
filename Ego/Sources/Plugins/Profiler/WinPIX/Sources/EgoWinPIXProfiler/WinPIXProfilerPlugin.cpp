#include "WinPIXProfilerPlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "WinPIXProfiler.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::winpix::WinPIXProfilerPlugin, ProfilerPlugin, ego::ProfilerPlugin);

ego::winpix::WinPIXProfilerPlugin::WinPIXProfilerPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : ProfilerPlugin(_module, _pluginType)
{
}

ego::profile::ProfilerPointer ego::winpix::WinPIXProfilerPlugin::createProfiler()
{
    return profile::ProfilerPointer(new WinPIXProfiler());
}
