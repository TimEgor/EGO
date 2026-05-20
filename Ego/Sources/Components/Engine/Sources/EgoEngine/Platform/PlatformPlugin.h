#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "Platform.h"

namespace ego
{
    class PlatformPlugin : public engine::EnginePlugin
    {
    public:
        PlatformPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual PlatformPointer createPlatform(void* _platformNativeInstance) = 0;

        EGO_PLUGIN(PlatformPlugin, engine::EnginePlugin);
    };

    EGO_POINTER(PlatformPlugin);
}
