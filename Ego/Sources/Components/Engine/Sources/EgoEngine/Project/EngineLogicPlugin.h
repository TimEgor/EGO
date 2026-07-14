#pragma once

#include "EgoPlugin/Plugin.h"

#include "EgoEngine/Project/EngineLogic.h"

namespace ego::engine
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
} // namespace ego::engine
