#pragma once

#include "EgoEngine/Project/EngineLogicPlugin.h"

namespace ego::demo
{
    class DebugDrawDemoPlugin final : public engine::EngineLogicPlugin
    {
    public:
        DebugDrawDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(DebugDrawDemoPlugin, engine::EngineLogicPlugin);
    };

    EGO_POINTER(DebugDrawDemoPlugin);
} // namespace ego::demo
