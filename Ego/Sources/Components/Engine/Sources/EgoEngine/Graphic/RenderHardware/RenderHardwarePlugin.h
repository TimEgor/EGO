#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "GraphicDevice.h"

namespace ego
{
    class ResourceController;

    class RenderHardwarePlugin : public engine::EnginePlugin
    {
    public:
        RenderHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual gpu::GraphicDeviceReference createGraphicDevice() = 0;
        virtual void registerResourceProviders(ResourceController&) {}

        EGO_PLUGIN(RenderHardwarePlugin, engine::EnginePlugin);
    };

    EGO_POINTER(RenderHardwarePlugin);
}
