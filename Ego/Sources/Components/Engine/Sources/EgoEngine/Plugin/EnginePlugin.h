#pragma once

#include "EgoPlugin/Plugin.h"

namespace ego::engine
{
    class EnginePlugin : public Plugin
    {
    public:
        EnginePlugin(const PluginModulePointer& _module)
            : Plugin(_module) {}

        EGO_PLUGIN(EnginePlugin, Plugin);
    };

    EGO_POINTER(EnginePlugin);
}
