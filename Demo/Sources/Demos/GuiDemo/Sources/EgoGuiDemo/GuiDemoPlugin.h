#pragma once

#include "EgoEngineFramework/Plugin/EngineLogicPlugin.h"

namespace ego::demo
{
    class GuiDemoPlugin final : public engine_framework::EngineLogicPlugin
    {
    public:
        GuiDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine_framework::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(GuiDemoPlugin, engine_framework::EngineLogicPlugin);
    };

    EGO_POINTER(GuiDemoPlugin);
} // namespace ego::demo
