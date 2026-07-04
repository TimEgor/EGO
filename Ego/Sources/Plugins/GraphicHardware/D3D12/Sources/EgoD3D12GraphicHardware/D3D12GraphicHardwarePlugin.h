#pragma once

#include "EgoGraphicHardware/GraphicHardwarePlugin.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicHardwarePlugin final : public GraphicHardwarePlugin
    {
    public:
        D3D12GraphicHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType);

        GraphicDevicePointer createGraphicDevice() override;

        EGO_PLUGIN(D3D12GraphicHardwarePlugin, GraphicHardwarePlugin);
    };

    EGO_POINTER(D3D12GraphicHardwarePlugin);
} // namespace ego::gpu::d3d12
