#pragma once

#include "EgoEngine/Resources/Resource/ResourceProviderPlugin.h"

namespace ego::resources::dxc
{
    class DXCResourceProviderPlugin final : public ResourceProviderPlugin
    {
    public:
        DXCResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        void registerResourceProviders() override;
        void unregisterResourceProviders() override;

        EGO_PLUGIN(DXCResourceProviderPlugin, ResourceProviderPlugin);
    };

    EGO_POINTER(DXCResourceProviderPlugin);
} // namespace ego::resources::dxc
