#pragma once

#include "EgoEngine/Project/EngineLogicPlugin.h"

namespace ego::demo
{
    class GuiDemoPlugin final : public engine::EngineLogicPlugin
    {
    public:
        GuiDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        engine::EngineLogicPointer createEngineLogic() override;

        EGO_PLUGIN(GuiDemoPlugin, engine::EngineLogicPlugin);
    };

    EGO_POINTER(GuiDemoPlugin);
} // namespace ego::demo
