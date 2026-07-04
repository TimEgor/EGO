#pragma once

#include "EgoEngineFramework/Plugin/EngineLogicPlugin.h"

namespace ego::demo
{
    class TestDemoPlugin final : public engine_framework::EngineLogicPlugin
    {
    public:
        TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine_framework::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(TestDemoPlugin, engine_framework::EngineLogicPlugin);
    };

    EGO_POINTER(TestDemoPlugin);
} // namespace ego::demo
