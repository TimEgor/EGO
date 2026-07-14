#pragma once

#include "EgoEngine/Project/EngineLogicPlugin.h"

namespace ego::demo
{
    class TestDemoPlugin final : public engine::EngineLogicPlugin
    {
    public:
        TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(TestDemoPlugin, engine::EngineLogicPlugin);
    };

    EGO_POINTER(TestDemoPlugin);
} // namespace ego::demo
