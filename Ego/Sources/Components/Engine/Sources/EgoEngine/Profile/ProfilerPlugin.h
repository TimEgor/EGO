#pragma once

#include "EgoCore/Profile/Profile.h"

#include "EgoEngine/Plugin/EnginePlugin.h"

namespace ego
{
    class ProfilerPlugin : public engine::EnginePlugin
    {
    public:
        ProfilerPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual void onLoaded() override;
        virtual void onUnloaded() override;

        virtual profile::ProfilerPointer createProfiler() = 0;

        EGO_PLUGIN(ProfilerPlugin, engine::EnginePlugin);

    private:
        profile::ProfilerPointer m_profiler = nullptr;
    };

    EGO_POINTER(ProfilerPlugin);
}
