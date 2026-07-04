#include "DXCResourceProviderPlugin.h"

#include "EgoRuntime/Plugin/ExternalModule.h"
#include "EgoRuntime/Resource/ResourceController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "DXCShaderResourceProvider.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::resources::dxc::DXCResourceProviderPlugin, ResourceProviderPlugin, ego::ResourceProviderPlugin);

namespace ego::resources::dxc
{
    DXCResourceProviderPlugin::DXCResourceProviderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
        : ResourceProviderPlugin(_module, _pluginType)
    {
    }

    void DXCResourceProviderPlugin::registerResourceProviders()
    {
        const DXCShaderResourceProviderPointer shaderProvider = new DXCShaderResourceProvider();
        ResourceController& resourceController = context::GetRuntimeContext().getResourceController();
        resourceController.addResourceProvider(".shader", shaderProvider);
        resourceController.addResourceProvider(".hlsl", shaderProvider);
    }

    void DXCResourceProviderPlugin::unregisterResourceProviders()
    {
        ResourceController& resourceController = context::GetRuntimeContext().getResourceController();
        resourceController.removeResourceProvider(".hlsl");
        resourceController.removeResourceProvider(".shader");
    }
} // namespace ego::resources::dxc
