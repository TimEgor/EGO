#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "GraphicDevice.h"

namespace ego
{
    class RenderHardwarePlugin : public engine::EnginePlugin
    {
    public:
        RenderHardwarePlugin(const PluginModulePointer& _module)
            : EnginePlugin(_module) {}

        virtual gpu::GraphicDevice* createGraphicDevice() = 0;

        EGO_PLUGIN(RenderHardwarePlugin, engine::EnginePlugin);
    };

    EGO_POINTER(RenderHardwarePlugin);
}
