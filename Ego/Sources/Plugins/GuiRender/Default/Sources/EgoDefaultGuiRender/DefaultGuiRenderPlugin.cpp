#include "DefaultGuiRenderPlugin.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"
#include "EgoGraphicHardware/Resources/ShaderResource.h"

#include "EgoPlugin/ExternalModule.h"
#include "EgoPlugin/PluginModule.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "DefaultGuiRenderConstants.h"
#include "DefaultGuiRenderFileSystems.h"

EGO_MODULE_ENTRY();

EGO_PLUGIN_CREATE(ego::gui::default_gui_render::DefaultGuiRenderPlugin, GuiRenderPlugin, ego::gui::GuiRenderPlugin);

ego::gui::default_gui_render::DefaultGuiRenderPlugin::DefaultGuiRenderPlugin(const PluginModulePointer& _module, PluginType _pluginType)
    : GuiRenderPlugin(_module, _pluginType)
{
}

ego::gui::GuiRenderPointer ego::gui::default_gui_render::DefaultGuiRenderPlugin::createGuiRender(GraphicDevice& _graphicDevice)
{
    const GraphicDevicePointer subsystemGraphicDevice = gpu::GetGraphicDevicePointer();
    EGO_CHECK_RETURN_NULL(subsystemGraphicDevice && subsystemGraphicDevice.get() == &_graphicDevice);

    const PluginModulePointer module = getModule();
    EGO_CHECK_RETURN_NULL(module);

    DefaultGuiRenderFileSystems fileSystems;
    FileName assetsRootPath;
    EGO_CHECK_RETURN_NULL(fileSystems.loadAssetsRootPath(module->getInfo().m_modulePath, assetsRootPath));
    EGO_CHECK_RETURN_NULL(fileSystems.initAssetsFileSystem(assetsRootPath));

    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    const ResourceControllerPointer resourceController = resourceSubsystem ? resourceSubsystem->getResourceControllerPointer() : nullptr;
    EGO_CHECK_RETURN_NULL(resourceController);

    const gpu::VertexShaderResourcePointer vertexShaderResource = resourceController->load<gpu::VertexShaderResource>(GuiVertexShaderPath);
    EGO_CHECK_RETURN_NULL(vertexShaderResource && vertexShaderResource->isLoaded());

    const gpu::PixelShaderResourcePointer pixelShaderResource = resourceController->load<gpu::PixelShaderResource>(GuiPixelShaderPath);
    EGO_CHECK_RETURN_NULL(pixelShaderResource && pixelShaderResource->isLoaded());

    GuiRenderPointer guiRender = new GuiRender();
    EGO_CHECK_RETURN_NULL(guiRender);

    GuiRender::InitData initData;
    initData.m_vertexShader = vertexShaderResource->getVertexShader();
    initData.m_pixelShader = pixelShaderResource->getPixelShader();
    EGO_CHECK_RETURN_NULL(guiRender->init(_graphicDevice, initData));

    return guiRender;
}
