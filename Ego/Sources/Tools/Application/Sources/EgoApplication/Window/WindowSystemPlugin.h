#pragma once

#include "EgoPlugin/Plugin.h"

#include "WindowSystem.h"

namespace ego
{
    class WindowSystemPlugin : public Plugin
    {
    public:
        WindowSystemPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType)
        {
        }

        virtual WindowSystemPointer createWindowSystem(void* _nativeInstanceHandle) = 0;

        EGO_PLUGIN(WindowSystemPlugin, Plugin);
    };

    EGO_POINTER(WindowSystemPlugin);
} // namespace ego
