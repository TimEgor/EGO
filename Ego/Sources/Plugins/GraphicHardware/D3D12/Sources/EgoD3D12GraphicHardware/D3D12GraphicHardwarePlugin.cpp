#include "D3D12GraphicHardwarePlugin.h"

#include "EgoPlugin/ExternalModule.h"

#include "D3D12GraphicDevice.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::gpu::d3d12::D3D12GraphicHardwarePlugin, GraphicHardwarePlugin, ego::gpu::GraphicHardwarePlugin);

ego::gpu::d3d12::D3D12GraphicHardwarePlugin::D3D12GraphicHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : GraphicHardwarePlugin(_module, _pluginType)
{
}

ego::GraphicDevicePointer ego::gpu::d3d12::D3D12GraphicHardwarePlugin::createGraphicDevice()
{
    return GraphicDevicePointer(new D3D12GraphicDevice());
}
