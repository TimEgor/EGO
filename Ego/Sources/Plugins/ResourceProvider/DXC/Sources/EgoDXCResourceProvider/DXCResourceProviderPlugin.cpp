#include "DXCResourceProviderPlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "DXCShaderResourceProvider.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::resources::dxc::DXCResourceProviderPlugin, ResourceProviderPlugin, ego::ResourceProviderPlugin);

namespace ego::resources::dxc
{
    DXCResourceProviderPlugin::DXCResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
        : ResourceProviderPlugin(_module, _pluginType)
    {
    }

    bool DXCResourceProviderPlugin::createRegistrations(RegistrationCollection& _registrations)
    {
        const DXCShaderResourceProviderPointer shaderProvider =
            MakePointer<DXCShaderResourceProvider>();
        if (!shaderProvider)
        {
            return false;
        }

        _registrations.push_back({".shader", shaderProvider});
        _registrations.push_back({".hlsl", shaderProvider});
        return true;
    }
} // namespace ego::resources::dxc
