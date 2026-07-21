#include "GuiRenderController.h"

#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoGui/Rendering/GuiRender.h"
#include "EgoGui/Rendering/GuiRenderPlugin.h"

ego::engine::GuiRenderController::~GuiRenderController()
{
    release();
}

bool ego::engine::GuiRenderController::init(const gui::GuiRenderPluginPointer& _guiRenderPlugin)
{
    const gui::GuiRenderPluginPointer guiRenderPlugin = _guiRenderPlugin;
    release();

    EGO_CHECK_INITIALIZATION(guiRenderPlugin);

    m_guiRenderPlugin = guiRenderPlugin;
    m_guiRender = m_guiRenderPlugin->createGuiRender();
    if (!m_guiRender || !m_guiRender->init())
    {
        release();
        return false;
    }

    return true;
}

void ego::engine::GuiRenderController::release()
{
    if (m_guiRender)
    {
        m_guiRender->release();
        m_guiRender = nullptr;
    }

    m_guiRenderPlugin = nullptr;
}

void ego::engine::GuiRenderController::clearResources()
{
    if (m_guiRender)
    {
        m_guiRender->clearResources();
    }
}

bool ego::engine::GuiRenderController::prepare(gui::GuiRenderData&& _renderData)
{
    EGO_CHECK_RETURN_FALSE(isInitialized());
    return m_guiRender->prepare(std::move(_renderData));
}

bool ego::engine::GuiRenderController::renderFrame(const FrameRenderTargetCollection& _targets)
{
    EGO_CHECK_RETURN_FALSE(isInitialized());

    if (_targets.empty())
    {
        return true;
    }

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();

    gui::GuiRender::TargetCollection targetViews;
    targetViews.reserve(_targets.size());
    for (const gpu::Texture2DReference& targetTexture : _targets)
    {
        EGO_CHECK_RETURN_FALSE(targetTexture);

        gpu::TextureViewDesc targetViewDesc;
        targetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
        targetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
        targetViewDesc.m_format = targetTexture->getDesc().m_format;

        const gpu::TextureViewReference targetView = graphicDevice.createTextureView(targetTexture, targetViewDesc);
        EGO_CHECK_RETURN_FALSE(targetView);
        targetViews.push_back(targetView);
    }

    return m_guiRender->render(targetViews);
}

bool ego::engine::GuiRenderController::isInitialized() const
{
    return m_guiRenderPlugin && m_guiRender;
}
