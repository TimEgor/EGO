#include "DefaultRenderPlugin.h"

#include "EgoCore/FileName/FileNameUtils.h"

#include "EgoEngine/Plugin/EngineExternalModuleCore.h"

#include "DefaultRender.h"

EGO_CORE_MODULE();
EGO_ENGINE_MODULE();

EGO_PLUGIN_CREATE(ego::render::default_render::DefaultRenderPlugin, RenderPlugin, ego::render::RenderPlugin);

namespace
{
    ego::FileName GetDefaultRenderRootPath(const ego::PluginModulePointer& _module)
    {
        if (!_module)
        {
            return ego::FileName();
        }

        return ego::file_name_utils::GetFileDirPath(_module->getInfo().m_modulePath);
    }
}

ego::render::default_render::DefaultRenderPlugin::DefaultRenderPlugin(
    const PluginModulePointer& _module,
    PluginType _pluginType
)
    : RenderPlugin(_module, _pluginType)
{}

ego::render::RenderPointer ego::render::default_render::DefaultRenderPlugin::createRender()
{
    return RenderPointer(new DefaultRender(GetDefaultRenderRootPath(getModule())));
}
