#pragma once

#include "EgoCore/Profile/ProfileController.h"

#include "EgoPlugin/Plugin.h"

namespace ego
{
    class ProfilerPlugin : public Plugin
    {
    public:
        ProfilerPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual profile::ProfilerPointer createProfiler() = 0;

        EGO_PLUGIN(ProfilerPlugin, Plugin);
    };

    EGO_POINTER(ProfilerPlugin);
} // namespace ego
