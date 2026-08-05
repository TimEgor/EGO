#pragma once

#include "EgoRuntime/Profile/ProfilerPlugin.h"

namespace ego::winpix
{
    class WinPIXProfilerPlugin final : public ProfilerPlugin
    {
    public:
        WinPIXProfilerPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        profile::ProfilerPointer createProfiler() override;

        EGO_PLUGIN(WinPIXProfilerPlugin, ProfilerPlugin);
    };

    EGO_POINTER(WinPIXProfilerPlugin);
} // namespace ego::winpix
