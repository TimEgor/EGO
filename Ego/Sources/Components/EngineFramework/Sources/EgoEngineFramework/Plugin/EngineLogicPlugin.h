#pragma once

#include "EgoRuntime/Plugin/Plugin.h"

#include "EgoEngineFramework/EngineLogic.h"

namespace ego::engine_framework
{
    class EngineLogicPlugin : public Plugin
    {
    public:
        EngineLogicPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual EngineLogicPointer createEngineLogic() = 0;

        EGO_PLUGIN(EngineLogicPlugin, Plugin)
    };

    EGO_POINTER(EngineLogicPlugin);
} // namespace ego::engine_framework
