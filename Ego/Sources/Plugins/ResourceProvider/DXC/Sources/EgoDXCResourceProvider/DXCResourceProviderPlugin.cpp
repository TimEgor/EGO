#include "DXCResourceProviderPlugin.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Plugin/EngineExternalModuleCore.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

#include "DXCShaderResourceProvider.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

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
        engine::GetEngine().getResourceController().addResourceProvider(".shader", shaderProvider);
        engine::GetEngine().getResourceController().addResourceProvider(".hlsl", shaderProvider);
    }

    void DXCResourceProviderPlugin::unregisterResourceProviders()
    {
        engine::GetEngine().getResourceController().removeResourceProvider(".hlsl");
        engine::GetEngine().getResourceController().removeResourceProvider(".shader");
    }
} // namespace ego::resources::dxc
