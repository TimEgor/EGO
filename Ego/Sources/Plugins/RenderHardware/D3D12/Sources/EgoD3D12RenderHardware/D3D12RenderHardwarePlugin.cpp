#include "D3D12RenderHardwarePlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

#include "D3D12GraphicDevice.h"
#include "DXCShaderResourceProvider.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::gpu::d3d12::D3D12RenderHardwarePlugin, RenderHardwarePlugin, ego::RenderHardwarePlugin);

ego::gpu::d3d12::D3D12RenderHardwarePlugin::D3D12RenderHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : RenderHardwarePlugin(_module, _pluginType)
{
    
}

ego::gpu::GraphicDeviceReference ego::gpu::d3d12::D3D12RenderHardwarePlugin::createGraphicDevice()
{
    return GraphicDeviceReference(new D3D12GraphicDevice());
}

void ego::gpu::d3d12::D3D12RenderHardwarePlugin::registerResourceProviders(ResourceController& _resourceController)
{
    const DXCShaderResourceProviderPointer shaderProvider = new DXCShaderResourceProvider();
    _resourceController.addResourceProvider(".shader", shaderProvider);
}
