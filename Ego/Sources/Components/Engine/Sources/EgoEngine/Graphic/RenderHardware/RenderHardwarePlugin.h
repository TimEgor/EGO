#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "GraphicDevice.h"

namespace ego
{
    class RenderHardwarePlugin : public engine::EnginePlugin
    {
    public:
        RenderHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual GraphicDevicePointer createGraphicDevice() = 0;

        EGO_PLUGIN(RenderHardwarePlugin, engine::EnginePlugin);
    };

    EGO_POINTER(RenderHardwarePlugin);
}
