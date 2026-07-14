#include "WinPIXProfilerPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "WinPIXProfiler.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::winpix::WinPIXProfilerPlugin, ProfilerPlugin, ego::ProfilerPlugin);

ego::winpix::WinPIXProfilerPlugin::WinPIXProfilerPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : ProfilerPlugin(_module, _pluginType)
{
}

ego::profile::ProfilerPointer ego::winpix::WinPIXProfilerPlugin::createProfiler()
{
    return profile::ProfilerPointer(new WinPIXProfiler());
}
