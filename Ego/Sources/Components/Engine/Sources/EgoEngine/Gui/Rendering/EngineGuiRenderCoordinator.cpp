#include "EngineGuiRenderCoordinator.h"

#include <algorithm>
#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoGui/Rendering/GuiFontAtlas.h"

#include "EgoEngine/Graphic/Render/Render.h"

ego::engine::EngineGuiRenderCoordinator::~EngineGuiRenderCoordinator()
{
    release();
}

bool ego::engine::EngineGuiRenderCoordinator::init(const EngineGuiViewportBackendPointer& _viewportBackend, const gui::GuiRenderPointer& _guiRender)
{
    release();

    EGO_CHECK_INITIALIZATION(_viewportBackend);
    EGO_CHECK_INITIALIZATION(_guiRender && _guiRender->isInitialized());

    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    EGO_CHECK_INITIALIZATION(graphicHardwareSubsystem);

    const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem->getGraphicDevicePointer();
    EGO_CHECK_INITIALIZATION(graphicDevice);

    m_commandQueue = graphicHardwareSubsystem->getGraphicCommandQueue();
    EGO_CHECK_INITIALIZATION(m_commandQueue && m_commandQueue->getCommandType() == gpu::CommandType::Graphic);

    m_frameFence = graphicDevice->createFence();
    EGO_CHECK_INITIALIZATION(m_frameFence);

    m_viewportBackend = _viewportBackend;
    m_guiRender = _guiRender;

    m_isInitialized = true;
    return true;
}

void ego::engine::EngineGuiRenderCoordinator::release()
{
    clearResources();

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_guiRender);
    m_frameFence = nullptr;
    m_commandQueue = nullptr;
    m_viewportBackend = nullptr;
    m_frameFenceValue = 0;
    m_primaryViewportID = gui::InvalidGuiViewportID;
    m_isInitialized = false;
}

void ego::engine::EngineGuiRenderCoordinator::clearResources()
{
    wait();

    m_presentedViewports.clear();
    m_preparedViewportIDs.clear();
    m_knownViewportIDs.clear();
    m_commandLists.clear();
    if (m_guiRender)
    {
        m_guiRender->clearResources();
    }
}

bool ego::engine::EngineGuiRenderCoordinator::prepare(gui::GuiFrame&& _frame)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);

    wait();

    gui::GuiViewportIDCollection viewportIDs;
    viewportIDs.reserve(_frame.m_viewports.size());
    for (const gui::GuiViewportFrame& viewportFrame : _frame.m_viewports)
    {
        viewportIDs.push_back(viewportFrame.m_viewportID);
    }
    removeUnusedViewportResources(viewportIDs);

    m_preparedViewportIDs.clear();
    m_primaryViewportID = _frame.m_primaryViewportID;

    for (gui::GuiViewportFrame& viewportFrame : _frame.m_viewports)
    {
        const gui::GuiViewportID viewportID = viewportFrame.m_viewportID;
        const EngineViewportHostPointer host = m_viewportBackend ? m_viewportBackend->findViewportHost(viewportID) : nullptr;
        if (!host)
        {
            continue;
        }

        const EngineViewportPrepareResult prepareResult = host->prepareForRender();
        if (prepareResult == EngineViewportPrepareResult::Failed)
        {
            if (viewportID == m_primaryViewportID)
            {
                return false;
            }

            continue;
        }

        if (prepareResult != EngineViewportPrepareResult::Ready)
        {
            continue;
        }

        EGO_CHECK_RETURN_FALSE(prepareViewport(std::move(viewportFrame), _frame.m_fontAtlas));
        m_preparedViewportIDs.push_back(viewportID);
    }

    return true;
}

bool ego::engine::EngineGuiRenderCoordinator::renderAndPresent(render::Render& _sceneRender)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);

    m_presentedViewports.clear();

    std::vector<gpu::CommandListReference> commandLists;
    commandLists.reserve(m_preparedViewportIDs.size());

    bool result = true;
    bool hasSubmittedWork = false;
    for (const gui::GuiViewportID viewportID : m_preparedViewportIDs)
    {
        const EngineViewportHostPointer host = m_viewportBackend ? m_viewportBackend->findViewportHost(viewportID) : nullptr;
        if (!host)
        {
            continue;
        }

        const GraphicPresenterPointer presenter = host->getGraphicPresenterPointer();
        const gpu::Texture2DReference target = presenter ? presenter->getTargetTexture() : nullptr;
        if (!presenter || !target)
        {
            continue;
        }

        bool hasScene = false;
        if (viewportID == m_primaryViewportID)
        {
            hasScene = _sceneRender.copyResultToTarget(target);
        }

        const size_t previousCommandListCount = commandLists.size();
        if (!recordViewport(viewportID, host, presenter, target, hasScene, commandLists))
        {
            result = false;
        }

        const bool hasGuiWork = commandLists.size() != previousCommandListCount;
        if (hasScene && !hasGuiWork)
        {
            PresentedViewport presentedViewport;
            presentedViewport.m_host = host;
            presentedViewport.m_presenter = presenter;
            presentedViewport.m_target = target;
            m_presentedViewports.push_back(presentedViewport);
        }

        hasSubmittedWork = hasSubmittedWork || hasScene || hasGuiWork;
    }

    if (!commandLists.empty() && m_commandQueue)
    {
        m_commandQueue->execute(commandLists);
    }

    for (const PresentedViewport& presentedViewport : m_presentedViewports)
    {
        if (presentedViewport.m_presenter)
        {
            presentedViewport.m_presenter->present();
        }
    }

    if (hasSubmittedWork)
    {
        signalFrameFence();
    }

    return result;
}

void ego::engine::EngineGuiRenderCoordinator::wait()
{
    if (m_frameFence)
    {
        m_frameFence->waitValue(m_frameFenceValue);
    }
    else if (m_commandQueue)
    {
        m_commandQueue->waitIdle();
    }

    m_presentedViewports.clear();
}

bool ego::engine::EngineGuiRenderCoordinator::isInitialized() const
{
    return m_isInitialized;
}

bool ego::engine::EngineGuiRenderCoordinator::prepareViewport(gui::GuiViewportFrame&& _viewportFrame, const gui::GuiFontAtlasPointer& _fontAtlas)
{
    gui::GuiRenderPacket packet;
    packet.m_viewportID = _viewportFrame.m_viewportID;
    packet.m_drawData = std::move(_viewportFrame.m_drawData);

    if (_fontAtlas && _fontAtlas->isInitialized())
    {
        gui::GuiRenderTextureBinding fontBinding;
        fontBinding.m_id = _fontAtlas->getTextureId();
        fontBinding.m_texture = _fontAtlas->getTexture();
        packet.m_textureBindings.push_back(fontBinding);
    }

    return m_guiRender && m_guiRender->prepare(gpu::GetGraphicDevice(), std::move(packet));
}

bool ego::engine::EngineGuiRenderCoordinator::recordViewport(
    gui::GuiViewportID _viewportID,
    const EngineViewportHostPointer& _host,
    const GraphicPresenterPointer& _presenter,
    const gpu::Texture2DReference& _target,
    bool _hasScene,
    std::vector<gpu::CommandListReference>& _commandLists)
{
    EGO_CHECK_RETURN_FALSE(_host && _presenter && _target);

    gpu::TextureViewDesc targetViewDesc;
    targetViewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    targetViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    targetViewDesc.m_format = _target->getDesc().m_format;

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    const gpu::TextureViewReference targetView = graphicDevice.createTextureView(_target, targetViewDesc);
    EGO_CHECK_RETURN_FALSE(targetView);

    const gpu::GraphicCommandListReference commandList = getOrCreateCommandList(_viewportID);
    EGO_CHECK_RETURN_FALSE(commandList);

    commandList->begin();

    gui::GuiRenderTarget renderTarget;
    renderTarget.m_texture = _target;
    renderTarget.m_renderTargetView = targetView;
    renderTarget.m_loadOperation = _hasScene ? gpu::AttachmentLoadOperation::Load : gpu::AttachmentLoadOperation::Clear;
    renderTarget.m_clearColor = FloatVector4Zero;

    const bool renderResult = m_guiRender && m_guiRender->record(graphicDevice, commandList, renderTarget, _viewportID);
    commandList->resourceBarrier(_target, gpu::GraphicResourceState::Present);
    commandList->end();

    PresentedViewport presentedViewport;
    presentedViewport.m_host = _host;
    presentedViewport.m_presenter = _presenter;
    presentedViewport.m_target = _target;
    presentedViewport.m_targetView = targetView;
    presentedViewport.m_commandList = commandList;
    m_presentedViewports.push_back(presentedViewport);

    _commandLists.push_back(commandList);
    return renderResult;
}

ego::gpu::GraphicCommandListReference ego::engine::EngineGuiRenderCoordinator::getOrCreateCommandList(gui::GuiViewportID _viewportID)
{
    const CommandListMap::const_iterator commandListIt = m_commandLists.find(_viewportID);
    if (commandListIt != m_commandLists.end())
    {
        return commandListIt->second;
    }

    const gpu::GraphicCommandListReference commandList = gpu::GetGraphicDevice().createGraphicCommandList();
    if (commandList)
    {
        m_commandLists.emplace(_viewportID, commandList);
    }

    return commandList;
}

void ego::engine::EngineGuiRenderCoordinator::removeUnusedViewportResources(const gui::GuiViewportIDCollection& _viewportIDs)
{
    for (const gui::GuiViewportID knownViewportID : m_knownViewportIDs)
    {
        if (std::find(_viewportIDs.begin(), _viewportIDs.end(), knownViewportID) == _viewportIDs.end())
        {
            if (m_guiRender)
            {
                m_guiRender->removeViewport(knownViewportID);
            }
        }
    }

    for (CommandListMap::iterator commandListIt = m_commandLists.begin(); commandListIt != m_commandLists.end();)
    {
        const bool isUsed = std::find(_viewportIDs.begin(), _viewportIDs.end(), commandListIt->first) != _viewportIDs.end();
        if (isUsed)
        {
            ++commandListIt;
            continue;
        }

        commandListIt = m_commandLists.erase(commandListIt);
    }

    m_knownViewportIDs = _viewportIDs;
}

void ego::engine::EngineGuiRenderCoordinator::signalFrameFence()
{
    if (!m_frameFence || !m_commandQueue)
    {
        return;
    }

    ++m_frameFenceValue;
    m_commandQueue->signal(m_frameFence, m_frameFenceValue);
}
