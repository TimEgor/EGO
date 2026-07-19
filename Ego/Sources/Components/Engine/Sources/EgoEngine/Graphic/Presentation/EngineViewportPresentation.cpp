#include "EngineViewportPresentation.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoGuiRender/GuiRender.h"

ego::engine::EngineViewportPresentation::~EngineViewportPresentation()
{
    release();
}

bool ego::engine::EngineViewportPresentation::init(const EngineViewportHostPointer& _host)
{
    release();

    EGO_CHECK_INITIALIZATION(_host);

    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    EGO_CHECK_INITIALIZATION(graphicHardwareSubsystem);

    const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem->getGraphicDevicePointer();
    EGO_CHECK_INITIALIZATION(graphicDevice);

    m_commandQueue = graphicHardwareSubsystem->getGraphicCommandQueue();
    EGO_CHECK_INITIALIZATION(m_commandQueue && m_commandQueue->getCommandType() == gpu::CommandType::Graphic);

    m_frameFence = graphicDevice->createFence();
    EGO_CHECK_INITIALIZATION(m_frameFence);

    for (FrameResources& resources : m_frameResources)
    {
        resources.m_commandList = graphicDevice->createGraphicCommandList();
        EGO_CHECK_INITIALIZATION(resources.m_commandList);
    }

    m_host = _host;
    m_isInitialized = true;
    return true;
}

void ego::engine::EngineViewportPresentation::release()
{
    wait();

    for (FrameResources& resources : m_frameResources)
    {
        resources = FrameResources();
    }

    m_frameFence = nullptr;
    m_commandQueue = nullptr;
    m_host = nullptr;
    m_frameFenceValue = 0;
    m_currentFrameIndex = InvalidFrameIndex;
    m_isInitialized = false;
}

ego::engine::EngineViewportPrepareResult ego::engine::EngineViewportPresentation::prepare()
{
    if (!m_isInitialized || !m_host)
    {
        return EngineViewportPrepareResult::Failed;
    }

    m_currentFrameIndex = m_currentFrameIndex == InvalidFrameIndex ? 0 : (m_currentFrameIndex + 1) % FrameResourceCount;

    FrameResources& resources = getCurrentFrameResources();
    waitFrame(resources);
    clearFrameResources(resources);

    const EngineViewportPrepareResult prepareResult = m_host->prepareForRender();
    if (prepareResult != EngineViewportPrepareResult::Ready)
    {
        return prepareResult;
    }

    return acquireTarget(resources) ? EngineViewportPrepareResult::Ready : EngineViewportPrepareResult::Failed;
}

bool ego::engine::EngineViewportPresentation::resizeTarget()
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized && m_host && m_currentFrameIndex != InvalidFrameIndex);

    wait();
    EGO_CHECK_RETURN_FALSE(m_host->resizeRenderTarget());
    return acquireTarget(getCurrentFrameResources());
}

bool ego::engine::EngineViewportPresentation::present(const SharedPointer<gui::GuiRender>& _guiRender, gui::ViewportID _viewportID, const gpu::Texture2DReference& _sceneTexture)
{
    if (!m_isInitialized || m_currentFrameIndex == InvalidFrameIndex)
    {
        return false;
    }

    FrameResources& resources = getCurrentFrameResources();
    EGO_CHECK_RETURN_FALSE(resources.m_presenter && resources.m_target);

    EGO_CHECK_RETURN_FALSE(prepareTargetView(resources));
    EGO_CHECK_RETURN_FALSE(prepareSceneTexture(resources, _sceneTexture));

    bool result = true;
    resources.m_commandList->begin();
    if (resources.m_sceneTexture)
    {
        recordSceneCopy(resources);
    }

    if (_guiRender)
    {
        gui::GuiRenderTarget renderTarget;
        renderTarget.m_texture = resources.m_target;
        renderTarget.m_renderTargetView = resources.m_targetView;
        renderTarget.m_loadOperation = resources.m_sceneTexture ? gpu::AttachmentLoadOperation::Load : gpu::AttachmentLoadOperation::Clear;
        renderTarget.m_clearColor = FloatVector4Zero;

        result = _guiRender->record(gpu::GetGraphicDevice(), resources.m_commandList, renderTarget, _viewportID, m_currentFrameIndex);
    }
    else if (!resources.m_sceneTexture)
    {
        recordClear(resources);
    }
    resources.m_commandList->resourceBarrier(resources.m_target, gpu::GraphicResourceState::Present);
    resources.m_commandList->end();

    m_commandQueue->execute(resources.m_commandList);
    signalFrameFence(resources);

    resources.m_presenter->present();
    return result;
}

void ego::engine::EngineViewportPresentation::wait()
{
    for (FrameResources& resources : m_frameResources)
    {
        waitFrame(resources);
        clearFrameResources(resources);
    }
}

ego::gpu::Texture2DReference ego::engine::EngineViewportPresentation::getTargetTexture() const
{
    return m_currentFrameIndex != InvalidFrameIndex ? getCurrentFrameResources().m_target : nullptr;
}

uint32_t ego::engine::EngineViewportPresentation::getFrameIndex() const
{
    return m_currentFrameIndex;
}

bool ego::engine::EngineViewportPresentation::matchesHost(const EngineViewportHostPointer& _host) const
{
    return m_host.get() == _host.get();
}

bool ego::engine::EngineViewportPresentation::isInitialized() const
{
    return m_isInitialized;
}

ego::engine::EngineViewportPresentation::FrameResources& ego::engine::EngineViewportPresentation::getCurrentFrameResources()
{
    EGO_ASSERT(m_currentFrameIndex < FrameResourceCount);
    return m_frameResources[m_currentFrameIndex];
}

const ego::engine::EngineViewportPresentation::FrameResources& ego::engine::EngineViewportPresentation::getCurrentFrameResources() const
{
    EGO_ASSERT(m_currentFrameIndex < FrameResourceCount);
    return m_frameResources[m_currentFrameIndex];
}

void ego::engine::EngineViewportPresentation::waitFrame(FrameResources& _resources)
{
    if (_resources.m_fenceValue != 0 && m_frameFence)
    {
        m_frameFence->waitValue(_resources.m_fenceValue);
    }

    _resources.m_fenceValue = 0;
}

void ego::engine::EngineViewportPresentation::clearFrameResources(FrameResources& _resources)
{
    _resources.m_targetView = nullptr;
    _resources.m_sceneTexture = nullptr;
    _resources.m_target = nullptr;
    _resources.m_presenter = nullptr;
}

bool ego::engine::EngineViewportPresentation::acquireTarget(FrameResources& _resources)
{
    _resources.m_presenter = m_host ? m_host->getGraphicPresenterPointer() : nullptr;
    _resources.m_target = _resources.m_presenter ? _resources.m_presenter->getTargetTexture() : nullptr;
    return _resources.m_presenter && _resources.m_target;
}

bool ego::engine::EngineViewportPresentation::prepareTargetView(FrameResources& _resources)
{
    EGO_CHECK_RETURN_FALSE(_resources.m_target);

    gpu::TextureViewDesc targetViewDesc;
    targetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    targetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    targetViewDesc.m_format = _resources.m_target->getDesc().m_format;

    _resources.m_targetView = gpu::GetGraphicDevice().createTextureView(_resources.m_target, targetViewDesc);
    return static_cast<bool>(_resources.m_targetView);
}

bool ego::engine::EngineViewportPresentation::prepareSceneTexture(FrameResources& _resources, const gpu::Texture2DReference& _sceneTexture)
{
    _resources.m_sceneTexture = nullptr;
    if (!_sceneTexture)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(_resources.m_target);

    const gpu::Texture2DDesc& sceneDesc = _sceneTexture->getDesc();
    const gpu::Texture2DDesc& targetDesc = _resources.m_target->getDesc();
    EGO_CHECK_RETURN_FALSE(sceneDesc.m_format == targetDesc.m_format);
    EGO_CHECK_RETURN_FALSE(sceneDesc.m_size.m_x == targetDesc.m_size.m_x && sceneDesc.m_size.m_y == targetDesc.m_size.m_y);

    _resources.m_sceneTexture = _sceneTexture;
    return true;
}

void ego::engine::EngineViewportPresentation::recordSceneCopy(FrameResources& _resources)
{
    EGO_ASSERT(_resources.m_commandList && _resources.m_sceneTexture && _resources.m_target);

    if (_resources.m_sceneTexture->getState() != gpu::GraphicResourceState::CopySrc)
    {
        _resources.m_commandList->resourceBarrier(_resources.m_sceneTexture, gpu::GraphicResourceState::CopySrc);
    }
    if (_resources.m_target->getState() != gpu::GraphicResourceState::CopyDst)
    {
        _resources.m_commandList->resourceBarrier(_resources.m_target, gpu::GraphicResourceState::CopyDst);
    }

    const gpu::Texture2DSize& targetSize = _resources.m_target->getDesc().m_size;
    gpu::TextureCopyRegionDesc copyRegion;
    copyRegion.m_extent = UInt32Vector3(targetSize.m_x, targetSize.m_y, 1);
    _resources.m_commandList->copyTexture(_resources.m_sceneTexture, _resources.m_target, copyRegion);
}

void ego::engine::EngineViewportPresentation::recordClear(FrameResources& _resources)
{
    EGO_ASSERT(_resources.m_commandList && _resources.m_target && _resources.m_targetView);

    if (_resources.m_target->getState() != gpu::GraphicResourceState::RenderTarget)
    {
        _resources.m_commandList->resourceBarrier(_resources.m_target, gpu::GraphicResourceState::RenderTarget);
    }

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _resources.m_targetView;
    colorAttachment.m_loadOperation = gpu::AttachmentLoadOperation::Clear;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = FloatVector4Zero;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = _resources.m_target->getDesc().m_size;

    _resources.m_commandList->beginRendering(renderingDesc);
    _resources.m_commandList->endRendering();
}

void ego::engine::EngineViewportPresentation::signalFrameFence(FrameResources& _resources)
{
    if (!m_frameFence || !m_commandQueue)
    {
        return;
    }

    ++m_frameFenceValue;
    m_commandQueue->signal(m_frameFence, m_frameFenceValue);
    _resources.m_fenceValue = m_frameFenceValue;
}
