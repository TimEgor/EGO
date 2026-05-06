#include "D3D12RenderHardwarePlugin.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "D3D12GraphicDevice.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::gpu::d3d12::D3D12RenderHardwarePlugin, RenderHardwarePlugin);

ego::gpu::d3d12::D3D12RenderHardwarePlugin::D3D12RenderHardwarePlugin(const PluginModulePointer& _module)
    : RenderHardwarePlugin(_module)
{
    
}

ego::gpu::GraphicDevice* ego::gpu::d3d12::D3D12RenderHardwarePlugin::createGraphicDevice()
{
    return new D3D12GraphicDevice();
}
