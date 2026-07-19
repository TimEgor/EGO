#include "DefaultRender.h"

#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "DefaultRenderConstants.h"

bool ego::render::DefaultRender::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    EGO_CHECK_INITIALIZATION(graphicDevice.getCapabilities().m_supportsBindlessResources);
    EGO_CHECK_INITIALIZATION(graphicDevice.getCapabilities().m_supportsRayTracing);

    EGO_CHECK_INITIALIZATION(m_frameExecutor.init(graphicDevice, gpu::GetGraphicCommandQueue()));

    FileName assetsRootPath;
    EGO_CHECK_INITIALIZATION(m_fileSystems.loadAssetsRootPath(assetsRootPath));
    EGO_CHECK_INITIALIZATION(m_fileSystems.initAssetsFileSystem(assetsRootPath));
    EGO_CHECK_INITIALIZATION(initPassGraph(graphicDevice));

    m_isInitialized = true;
    return m_isInitialized;
}

void ego::render::DefaultRender::release()
{
    clearResources();

    m_shaderData.release();
    releasePassGraph();
    m_pipelineStateCache.clear();
    m_fileSystems.release();
    m_renderTarget.release();
    m_frameExecutor.release();
    m_isPrepared = false;
    m_isInitialized = false;
}

void ego::render::DefaultRender::clearResources()
{
    wait();

    m_scene.clear();
    m_shaderData.clearResources();
    m_passGraph.clearResources();
    m_pipelineStateCache.releaseUnused();
    m_isPrepared = false;
}

bool ego::render::DefaultRender::prepare(const RenderPrepareContext& _context)
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL();
        return false;
    }

    if (!m_frameExecutor.isValid())
    {
        EGO_ASSERT_FAIL();
        return false;
    }

    wait();
    m_isPrepared = false;

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    if (!m_renderTarget.prepare(graphicDevice, m_pendingResolution, DefaultRenderTargetFormat))
    {
        handlePrepareFailure();
        return false;
    }

    m_scene.collect(_context.m_level);

    if (!m_shaderData.prepare(graphicDevice, _context.m_level, _context.m_cameraEntity, m_scene, m_renderTarget.getResolution()))
    {
        handlePrepareFailure();
        return false;
    }

    RenderPassPrepareContext passContext{graphicDevice, m_renderTarget, m_scene, m_shaderData, m_settings, _context.m_deltaTime};
    if (!m_passGraph.prepare(passContext))
    {
        handlePrepareFailure();
        return false;
    }

    m_isPrepared = true;
    return true;
}

void ego::render::DefaultRender::render()
{
    if (!m_isInitialized)
    {
        EGO_ASSERT_FAIL();
        return;
    }

    if (!m_isPrepared)
    {
        return;
    }

    const std::vector<RenderGraphicCommandList>& commandLists = m_passGraph.getCommandLists();
    if (!m_frameExecutor.isValid() || commandLists.empty() || !m_renderTarget.isReady())
    {
        EGO_ASSERT_FAIL();
        m_passGraph.clearResources();
        m_pipelineStateCache.releaseUnused();
        return;
    }

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    RenderPassExecuteContext passContext{graphicDevice, m_pipelineStateCache, commandLists.front(), m_renderTarget, m_scene, m_shaderData, m_settings};
    const bool passExecutionResult = m_passGraph.execute(
        passContext,
        [this](const RenderGraphicCommandList& _commandList)
        {
            m_renderTarget.transition(_commandList, gpu::GraphicResourceState::CopySrc);
        });
    if (!passExecutionResult)
    {
        EGO_ASSERT_FAIL();
        m_passGraph.clearResources();
        m_pipelineStateCache.releaseUnused();
        m_isPrepared = false;
        return;
    }

    m_frameExecutor.submitCommandLists(commandLists);
    m_isPrepared = false;
}

void ego::render::DefaultRender::wait()
{
    m_frameExecutor.wait();
}

ego::gpu::Texture2DReference ego::render::DefaultRender::getResultTexture() const
{
    return m_renderTarget.getTexture().getObject();
}

void ego::render::DefaultRender::setResolution(const gpu::Texture2DSize& _resolution)
{
    if (_resolution.m_x == 0 || _resolution.m_y == 0)
    {
        EGO_ASSERT_FAIL();
        return;
    }

    m_pendingResolution = _resolution;
}

const ego::gpu::Texture2DSize& ego::render::DefaultRender::getResolution() const
{
    return m_pendingResolution;
}

void ego::render::DefaultRender::drawPoint(const DebugDrawPointData& _point)
{
    m_debugPass.drawPoint(_point);
}

void ego::render::DefaultRender::drawLine(const DebugDrawLineData& _line)
{
    m_debugPass.drawLine(_line);
}

bool ego::render::DefaultRender::isInitialized() const
{
    return m_isInitialized;
}

void ego::render::DefaultRender::setClearColor(const FloatVector4& _clearColor)
{
    m_settings.m_clearColor = _clearColor;
}

const ego::FloatVector4& ego::render::DefaultRender::getClearColor() const
{
    return m_settings.m_clearColor;
}

void ego::render::DefaultRender::setClearEnabled(bool _enabled)
{
    m_settings.m_clearEnabled = _enabled;
}

bool ego::render::DefaultRender::isClearEnabled() const
{
    return m_settings.m_clearEnabled;
}

bool ego::render::DefaultRender::initPassGraph(GraphicDevice& _graphicDevice)
{
    m_passGraph.clear();
    m_passGraph.addPass("Clear", m_clearPass);
    m_passGraph.addPass("RayTracing", m_rayTracingPass);
    m_passGraph.addPass("Debug", m_debugPass);
    EGO_CHECK_INITIALIZATION(m_passGraph.compile());
    EGO_CHECK_INITIALIZATION(m_passGraph.prepareCommandLists(_graphicDevice));

    RenderPassInitContext passContext{_graphicDevice, DefaultRenderTargetFormat};
    return m_passGraph.init(passContext);
}

void ego::render::DefaultRender::releasePassGraph()
{
    m_passGraph.release();
    m_passGraph.clear();
}

void ego::render::DefaultRender::handlePrepareFailure()
{
    m_scene.clear();
    m_passGraph.clearResources();
    m_pipelineStateCache.releaseUnused();
}
