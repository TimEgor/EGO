#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "Platform.h"

namespace ego
{
    class PlatformPlugin : public engine::EnginePlugin
    {
    public:
        PlatformPlugin(const PluginModulePointer& _module)
            : EnginePlugin(_module) {}

        virtual Platform* createPlatform(void* _platformNativeInstance) = 0;

        EGO_PLUGIN(PlatformPlugin, engine::EnginePlugin);
    };

    EGO_POINTER(PlatformPlugin);
}
