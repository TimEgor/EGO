#pragma once

#include "EgoEngine/Graphic/Render/RenderPlugin.h"

namespace ego::default_render
{
    class DefaultRenderPlugin final : public RenderPlugin
    {
    public:
        DefaultRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType);

        virtual RenderPointer createRender() override;

        EGO_PLUGIN(DefaultRenderPlugin, RenderPlugin);
    };

    EGO_POINTER(DefaultRenderPlugin);
}
