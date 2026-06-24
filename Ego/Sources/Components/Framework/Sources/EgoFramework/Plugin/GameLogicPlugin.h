#pragma once

#include "EgoPlugin/Plugin.h"

#include "EgoFramework/GameLogic.h"

namespace ego::framework
{
    class GameLogicPlugin : public Plugin
    {
    public:
        GameLogicPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual GameLogicPointer createGameLogic() = 0;

        EGO_PLUGIN(GameLogicPlugin, Plugin)
    };

    EGO_POINTER(GameLogicPlugin);
} // namespace ego::framework
