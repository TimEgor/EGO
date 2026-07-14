#pragma once

#include "EgoResource/ResourceProviderPlugin.h"

namespace ego::resources::dxc
{
    class DXCResourceProviderPlugin final : public ResourceProviderPlugin
    {
    public:
        DXCResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        bool createRegistrations(RegistrationCollection& _registrations) override;

        EGO_PLUGIN(DXCResourceProviderPlugin, ResourceProviderPlugin);
    };

    EGO_POINTER(DXCResourceProviderPlugin);
} // namespace ego::resources::dxc
