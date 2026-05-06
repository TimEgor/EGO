#pragma once

#include "EgoEngine/RenderHardware/RenderHardwarePlugin.h"

namespace ego::gpu::d3d12
{
    class D3D12RenderHardwarePlugin final : public RenderHardwarePlugin
    {
    public:
        D3D12RenderHardwarePlugin(const PluginModulePointer& _module);

        virtual GraphicDevice* createGraphicDevice() override;

        EGO_PLUGIN(D3D12RenderHardwarePlugin, RenderHardwarePlugin);
    };

    EGO_POINTER(D3D12RenderHardwarePlugin);
}
