#pragma once

#include "EgoEngineFramework/Plugin/EngineLogicPlugin.h"

namespace ego::demo
{
    class DebugDrawDemoPlugin final : public engine_framework::EngineLogicPlugin
    {
    public:
        DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine_framework::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(DebugDrawDemoPlugin, engine_framework::EngineLogicPlugin);
    };

    EGO_POINTER(DebugDrawDemoPlugin);
} // namespace ego::demo
