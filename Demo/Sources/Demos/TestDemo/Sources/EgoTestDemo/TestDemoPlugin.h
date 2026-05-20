#pragma once

#include "EgoDemoFramework/PluginController/DemoPlugin.h"

namespace ego::demo
{
    class TestDemoPlugin final : public DemoPlugin
    {
    public:
        TestDemoPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        DemoPointer createDemo() override;

        EGO_PLUGIN(TestDemoPlugin, DemoPlugin);
    };

    EGO_POINTER(TestDemoPlugin);
}
