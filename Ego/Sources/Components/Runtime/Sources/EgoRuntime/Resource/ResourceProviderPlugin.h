#pragma once

#include "EgoRuntime/Plugin/Plugin.h"

namespace ego
{
    class ResourceProviderPlugin : public Plugin
    {
    public:
        ResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual void registerResourceProviders() = 0;
        virtual void unregisterResourceProviders() = 0;

        EGO_PLUGIN(ResourceProviderPlugin, Plugin);
    };

    EGO_POINTER(ResourceProviderPlugin);
} // namespace ego
