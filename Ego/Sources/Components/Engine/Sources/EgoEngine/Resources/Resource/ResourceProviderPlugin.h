#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

namespace ego
{
    class ResourceProviderPlugin : public engine::EnginePlugin
    {
    public:
        ResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual void registerResourceProviders() = 0;
        virtual void unregisterResourceProviders() = 0;

        EGO_PLUGIN(ResourceProviderPlugin, engine::EnginePlugin);
    };

    EGO_POINTER(ResourceProviderPlugin);
}
