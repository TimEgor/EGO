#pragma once

#include "EgoPlugin/Plugin.h"

#include "EgoDemoFramework/Demo.h"

namespace ego::demo
{
    class DemoPlugin : public Plugin
    {
    public:
        DemoPlugin(const PluginModulePointer& _module, PluginType _pluginType)
            : Plugin(_module, _pluginType) {}

        virtual DemoPointer createDemo() = 0;

        EGO_PLUGIN(DemoPlugin, Plugin)
    };

    EGO_POINTER(DemoPlugin);
}
