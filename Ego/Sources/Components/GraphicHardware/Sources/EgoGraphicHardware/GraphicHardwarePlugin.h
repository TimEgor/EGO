#pragma once

#include "EgoPlugin/Plugin.h"

#include "GraphicDevice.h"

namespace ego::gpu
{
    class GraphicHardwarePlugin : public Plugin
    {
    public:
        GraphicHardwarePlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual GraphicDevicePointer createGraphicDevice() = 0;

        EGO_PLUGIN(GraphicHardwarePlugin, Plugin);
    };

    EGO_POINTER(GraphicHardwarePlugin);
} // namespace ego::gpu
