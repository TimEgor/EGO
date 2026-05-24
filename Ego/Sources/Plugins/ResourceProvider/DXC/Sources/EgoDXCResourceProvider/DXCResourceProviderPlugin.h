#pragma once

#include "EgoEngine/Resources/Resource/ResourceProviderPlugin.h"

namespace ego::resources::dxc
{
    class DXCResourceProviderPlugin final : public ResourceProviderPlugin
    {
    public:
        DXCResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        virtual void registerResourceProviders() override;
        virtual void unregisterResourceProviders() override;

        EGO_PLUGIN(DXCResourceProviderPlugin, ResourceProviderPlugin);
    };

    EGO_POINTER(DXCResourceProviderPlugin);
}
