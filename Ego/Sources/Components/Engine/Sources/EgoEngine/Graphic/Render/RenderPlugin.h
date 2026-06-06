#pragma once

#include "EgoEngine/Plugin/EnginePlugin.h"

#include "Render.h"

namespace ego::render
{
    class RenderPlugin : public engine::EnginePlugin
    {
    public:
        RenderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : EnginePlugin(_module, _pluginType) {}

        virtual RenderPointer createRender() = 0;

        EGO_PLUGIN(RenderPlugin, engine::EnginePlugin);
    };

    EGO_POINTER(RenderPlugin);
}
