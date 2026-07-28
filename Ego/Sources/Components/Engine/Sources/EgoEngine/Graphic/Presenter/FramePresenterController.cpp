#include "FramePresenterController.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

ego::engine::FramePresenterController::~FramePresenterController()
{
    release();
}

bool ego::engine::FramePresenterController::init(bool _enablePresentation)
{
    release();

    if (!_enablePresentation)
    {
        return true;
    }

    EGO_CHECK_INITIALIZATION(initPresentation());
    return true;
}

void ego::engine::FramePresenterController::release()
{
    clearResources();
    releasePresentation();
}

void ego::engine::FramePresenterController::clearResources()
{
    waitCommandList();
    m_targetViews.clear();
    m_graphicPresenters.clear();
}

void ego::engine::FramePresenterController::prepareFrame(const std::vector<GraphicPresenterPointer>& _graphicPresenters)
{
    waitCommandList();
    m_targetViews.clear();
    m_graphicPresenters.clear();

    m_graphicPresenters.reserve(_graphicPresenters.size());

    for (const GraphicPresenterPointer& graphicPresenter : _graphicPresenters)
    {
        if (!graphicPresenter)
        {
            continue;
        }

        const std::pair<GraphicPresenterCollection::iterator, bool> insertResult = m_graphicPresenters.insert(graphicPresenter);
        if (!insertResult.second)
        {
            continue;
        }

        const bool prepareResult = graphicPresenter->prepare();
        EGO_ASSERT_MESSAGE(prepareResult, "Failed to prepare a graphic presenter.");
        if (!prepareResult)
        {
            m_graphicPresenters.erase(insertResult.first);
        }
    }

    clearPresenters();
}

void ego::engine::FramePresenterController::presentFrame()
{
    if (!m_isPresentationEnabled)
    {
        return;
    }

    transitionPresenterTargets();

    for (const GraphicPresenterPointer& graphicPresenter : m_graphicPresenters)
    {
        graphicPresenter->present();
    }
}

ego::gpu::Texture2DPointer ego::engine::FramePresenterController::getTargetTexture(const GraphicPresenterPointer& _graphicPresenter) const
{
    return containsPresenter(_graphicPresenter) ? _graphicPresenter->getTargetTexture() : nullptr;
}

bool ego::engine::FramePresenterController::containsPresenter(const GraphicPresenterPointer& _graphicPresenter) const
{
    return _graphicPresenter && m_graphicPresenters.find(_graphicPresenter) != m_graphicPresenters.end();
}

bool ego::engine::FramePresenterController::initPresentation()
{
    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    EGO_CHECK_RETURN_FALSE(graphicHardwareSubsystem);

    const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem->getGraphicDevicePointer();
    EGO_CHECK_RETURN_FALSE(graphicDevice);

    m_presentationQueue = graphicHardwareSubsystem->getGraphicCommandQueue();
    EGO_CHECK_RETURN_FALSE(m_presentationQueue && m_presentationQueue->getCommandType() == gpu::CommandType::Graphic);

    m_commandList = graphicDevice->createGraphicCommandList();
    EGO_CHECK_RETURN_FALSE(m_commandList);

    m_fence = graphicDevice->createFence();
    EGO_CHECK_RETURN_FALSE(m_fence);

    m_isPresentationEnabled = true;
    return true;
}

void ego::engine::FramePresenterController::releasePresentation()
{
    m_fence = nullptr;
    m_commandList = nullptr;
    m_presentationQueue = nullptr;
    m_fenceValue = 0;
    m_isPresentationEnabled = false;
}

void ego::engine::FramePresenterController::clearPresenters()
{
    if (!m_isPresentationEnabled || m_graphicPresenters.empty())
    {
        return;
    }

    EGO_ASSERT(m_commandList && m_presentationQueue && m_fence);

    bool hasRecordedClear = false;
    m_targetViews.reserve(m_graphicPresenters.size());

    m_commandList->begin();

    for (const GraphicPresenterPointer& graphicPresenter : m_graphicPresenters)
    {
        if (!graphicPresenter->shouldClearTarget())
        {
            continue;
        }

        const gpu::Texture2DPointer targetTexture = graphicPresenter->getTargetTexture();
        if (!targetTexture || targetTexture->getDesc().m_size.m_x == 0 || targetTexture->getDesc().m_size.m_y == 0)
        {
            continue;
        }

        hasRecordedClear = recordTargetClear(targetTexture) || hasRecordedClear;
    }

    m_commandList->end();

    if (hasRecordedClear)
    {
        m_presentationQueue->execute(m_commandList);
        signalFence();
    }
}

bool ego::engine::FramePresenterController::recordTargetClear(const gpu::Texture2DPointer& _targetTexture)
{
    EGO_CHECK_RETURN_FALSE(m_commandList && _targetTexture);

    gpu::TextureViewDesc targetViewDesc;
    targetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    targetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    targetViewDesc.m_format = _targetTexture->getDesc().m_format;

    const gpu::TextureViewPointer targetView = gpu::GetGraphicDevice().createTextureView(_targetTexture, targetViewDesc);
    EGO_CHECK_RETURN_FALSE(targetView);
    m_targetViews.push_back(targetView);

    if (_targetTexture->getState() != gpu::GraphicResourceState::RenderTarget)
    {
        m_commandList->resourceBarrier(_targetTexture, gpu::GraphicResourceState::RenderTarget);
    }

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = targetView;
    colorAttachment.m_loadOperation = gpu::AttachmentLoadOperation::Clear;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = FloatVector4(0.0f, 0.0f, 0.0f, 1.0f);

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = _targetTexture->getDesc().m_size;

    m_commandList->beginRendering(renderingDesc);
    m_commandList->endRendering();

    return true;
}

void ego::engine::FramePresenterController::transitionPresenterTargets()
{
    EGO_ASSERT(m_commandList && m_presentationQueue && m_fence);

    waitCommandList();

    bool hasRecordedTransition = false;
    m_commandList->begin();

    for (const GraphicPresenterPointer& graphicPresenter : m_graphicPresenters)
    {
        const gpu::Texture2DPointer targetTexture = graphicPresenter->getTargetTexture();
        const gpu::GraphicResourceState presentationState = graphicPresenter->getPresentationState();
        if (targetTexture && targetTexture->getState() != presentationState)
        {
            m_commandList->resourceBarrier(targetTexture, presentationState);
            hasRecordedTransition = true;
        }
    }

    m_commandList->end();

    if (hasRecordedTransition)
    {
        m_presentationQueue->execute(m_commandList);
        signalFence();
    }
}

void ego::engine::FramePresenterController::waitCommandList()
{
    if (m_fence && m_fenceValue != 0)
    {
        m_fence->waitValue(m_fenceValue);
    }
}

void ego::engine::FramePresenterController::signalFence()
{
    EGO_ASSERT(m_fence && m_presentationQueue);

    ++m_fenceValue;
    m_presentationQueue->signal(m_fence, m_fenceValue);
}
