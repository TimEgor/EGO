#pragma once

#include "EgoFramework/Plugin/GameLogicPlugin.h"

namespace ego::demo
{
    class DebugDrawDemoPlugin final : public framework::GameLogicPlugin
    {
    public:
        DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        framework::GameLogicPointer createGameLogic() override;

        EGO_PLUGIN(DebugDrawDemoPlugin, framework::GameLogicPlugin);
    };

    EGO_POINTER(DebugDrawDemoPlugin);
}
