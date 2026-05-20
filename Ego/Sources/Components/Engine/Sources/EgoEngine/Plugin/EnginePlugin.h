#pragma once

#include "EgoPlugin/Plugin.h"

namespace ego::engine
{
    class EnginePlugin : public Plugin
    {
    public:
        EnginePlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType) {}

        EGO_PLUGIN(EnginePlugin, Plugin);
    };

    EGO_POINTER(EnginePlugin);
}
