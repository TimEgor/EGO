#include "GraphicFrameController.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoEngine/Level/Level.h"

ego::engine::GraphicFrameController::~GraphicFrameController()
{
    release();
}

bool ego::engine::GraphicFrameController::init(const InitData& _initData)
{
    release();

    EGO_CHECK_INITIALIZATION(!_initData.m_guiRenderPlugin || _initData.m_enablePresentation);

    EGO_CHECK_INITIALIZATION(m_framePresenterController.init(_initData.m_enablePresentation));

    if (_initData.m_sceneRenderPlugin)
    {
        EGO_CHECK_INITIALIZATION(m_sceneRenderController.init(_initData.m_sceneRenderPlugin));
    }
    if (_initData.m_guiRenderPlugin)
    {
        EGO_CHECK_INITIALIZATION(m_guiRenderController.init(_initData.m_guiRenderPlugin));
    }

    return true;
}

void ego::engine::GraphicFrameController::release()
{
    clearResources();

    m_guiRenderController.release();
    m_sceneRenderController.release();
    m_framePresenterController.release();
}

void ego::engine::GraphicFrameController::clearResources()
{
    m_guiRenderController.clearResources();
    m_sceneRenderController.clearResources();
    m_framePresenterController.clearResources();
    m_preparedGuiPresenters.clear();
    m_sceneGraphicPresenter = nullptr;
}

void ego::engine::GraphicFrameController::prepareFrame(gui::GuiRenderData&& _guiRenderData, const SceneRenderData& _sceneRenderData)
{
    m_sceneRenderController.wait();
    m_guiRenderController.clearResources();
    m_preparedGuiPresenters.clear();

    m_sceneGraphicPresenter = _sceneRenderData.m_graphicPresenter;

    std::vector<GraphicPresenterPointer> graphicPresenters;
    graphicPresenters.reserve(_guiRenderData.m_viewports.size() + (m_sceneGraphicPresenter ? 1 : 0));
    if (m_sceneGraphicPresenter)
    {
        graphicPresenters.push_back(m_sceneGraphicPresenter);
    }

    for (const gui::ViewportRenderData& viewportFrame : _guiRenderData.m_viewports)
    {
        if (viewportFrame.m_graphicPresenter)
        {
            graphicPresenters.push_back(viewportFrame.m_graphicPresenter);
        }
    }

    m_framePresenterController.prepareFrame(graphicPresenters);
    prepareGuiFrame(std::move(_guiRenderData));
    prepareSceneFrame(_sceneRenderData);
}

void ego::engine::GraphicFrameController::renderFrame()
{
    const gpu::Texture2DPointer targetTexture = m_framePresenterController.getTargetTexture(m_sceneGraphicPresenter);
    if (!targetTexture)
    {
        m_sceneRenderController.render(nullptr);
        return;
    }

    gpu::TextureViewDesc targetViewDesc;
    targetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    targetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    targetViewDesc.m_format = targetTexture->getDesc().m_format;

    const gpu::TextureViewPointer targetView = gpu::GetGraphicDevice().createTextureView(targetTexture, targetViewDesc);
    EGO_ASSERT(targetView);
    m_sceneRenderController.render(targetView);
}

void ego::engine::GraphicFrameController::presentFrame()
{
    renderGuiFrame();
    m_framePresenterController.presentFrame();
}

ego::render::Render& ego::engine::GraphicFrameController::getRender()
{
    return m_sceneRenderController.getRender();
}

void ego::engine::GraphicFrameController::prepareGuiFrame(gui::GuiRenderData&& _guiRenderData)
{
    if (!m_guiRenderController.isInitialized())
    {
        return;
    }

    gui::GuiRenderData filteredRenderData;
    filteredRenderData.m_resourceTextureViews = std::move(_guiRenderData.m_resourceTextureViews);
    filteredRenderData.m_viewports.reserve(_guiRenderData.m_viewports.size());
    m_preparedGuiPresenters.reserve(_guiRenderData.m_viewports.size());
    for (gui::ViewportRenderData& viewportRenderData : _guiRenderData.m_viewports)
    {
        const GraphicPresenterPointer graphicPresenter = viewportRenderData.m_graphicPresenter;
        const FloatVector2& viewportSize = viewportRenderData.m_drawData.m_viewportSize;
        const gpu::Texture2DPointer targetTexture = m_framePresenterController.getTargetTexture(graphicPresenter);
        if (!targetTexture || viewportSize.m_x <= 0.0f || viewportSize.m_y <= 0.0f)
        {
            continue;
        }

        filteredRenderData.m_viewports.push_back(std::move(viewportRenderData));
        m_preparedGuiPresenters.push_back(graphicPresenter);
    }

    const bool prepareResult = m_guiRenderController.prepare(std::move(filteredRenderData));
    EGO_ASSERT(prepareResult);
    if (!prepareResult)
    {
        m_preparedGuiPresenters.clear();
    }
}

void ego::engine::GraphicFrameController::prepareSceneFrame(const SceneRenderData& _sceneRenderData)
{
    if (!m_sceneRenderController.isInitialized())
    {
        return;
    }

    if (!_sceneRenderData.m_activeLevel || !_sceneRenderData.m_cameraEntity)
    {
        return;
    }

    const gpu::Texture2DPointer targetTexture = m_framePresenterController.getTargetTexture(m_sceneGraphicPresenter);
    if (!targetTexture)
    {
        return;
    }

    const render::RenderPrepareContext prepareContext{*_sceneRenderData.m_activeLevel,
        _sceneRenderData.m_cameraEntity,
        targetTexture->getDesc().m_size,
        _sceneRenderData.m_deltaTime};
    const bool prepareResult = m_sceneRenderController.prepare(prepareContext);
    EGO_ASSERT(prepareResult);
}

void ego::engine::GraphicFrameController::renderGuiFrame()
{
    if (!m_guiRenderController.isInitialized())
    {
        return;
    }

    GuiRenderController::FrameRenderTargetCollection targetTextures;
    targetTextures.reserve(m_preparedGuiPresenters.size());
    for (const GraphicPresenterPointer& graphicPresenter : m_preparedGuiPresenters)
    {
        targetTextures.push_back(m_framePresenterController.getTargetTexture(graphicPresenter));
    }

    const bool renderResult = m_guiRenderController.renderFrame(targetTextures);
    EGO_ASSERT(renderResult);
}
