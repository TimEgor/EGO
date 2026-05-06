#pragma once

#include "EgoPlugin/Plugin.h"

namespace ego::demo
{
    class DemoPlugin : public Plugin
    {
    public:
        DemoPlugin() = default;

        EGO_PLUGIN(DemoPlugin, Plugin)
    };

    EGO_POINTER(DemoPlugin);
}
