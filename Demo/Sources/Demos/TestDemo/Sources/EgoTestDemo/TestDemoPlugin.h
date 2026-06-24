#pragma once

#include "EgoFramework/Plugin/GameLogicPlugin.h"

namespace ego::demo
{
    class TestDemoPlugin final : public framework::GameLogicPlugin
    {
    public:
        TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        framework::GameLogicPointer createGameLogic() override;

        EGO_PLUGIN(TestDemoPlugin, framework::GameLogicPlugin);
    };

    EGO_POINTER(TestDemoPlugin);
} // namespace ego::demo
