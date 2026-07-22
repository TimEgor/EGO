#include "DefaultRender.h"

#include <utility>
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

    if (_context.m_targetSize.m_x == 0 || _context.m_targetSize.m_y == 0)
    {
        handlePrepareFailure();
        return false;
    }

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    if (!m_renderTarget.prepare(graphicDevice, _context.m_targetSize, DefaultRenderTargetFormat))
    {
        handlePrepareFailure();
        return false;
    }

    m_scene.collect(_context.m_level);

    if (!m_shaderData.prepare(graphicDevice, _context.m_level, _context.m_cameraEntity, m_scene, _context.m_targetSize))
    {
        handlePrepareFailure();
        return false;
    }

    RenderPassPrepareContext passContext{graphicDevice, m_scene, m_shaderData, m_settings, _context.m_deltaTime};
    if (!m_passGraph.prepare(passContext))
    {
        handlePrepareFailure();
        return false;
    }

    m_isPrepared = true;
    return true;
}

void ego::render::DefaultRender::render(const gpu::TextureViewReference& _targetView)
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

    const gpu::Texture2DReference targetTexture = resolvePresentationTargetTexture(_targetView);
    if (!targetTexture)
    {
        EGO_ASSERT_FAIL();
        m_isPrepared = false;
        return;
    }

    const std::vector<RenderGraphicCommandList>& commandLists = m_passGraph.getCommandLists();
    if (!m_frameExecutor.isValid() || commandLists.empty())
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
        [this, &targetTexture](const RenderGraphicCommandList& _commandList)
        {
            copyResultToTarget(_commandList, targetTexture);
        });
    if (!passExecutionResult)
    {
        EGO_ASSERT_FAIL();
        m_passGraph.clearResources();
        m_pipelineStateCache.releaseUnused();
        m_isPrepared = false;
        return;
    }

    std::vector<gpu::GraphicObjectReference> frameResources;
    frameResources.reserve(4);
    frameResources.push_back(m_renderTarget.getTexture().getObject());
    frameResources.push_back(m_renderTarget.getRenderTargetView().getObject());
    frameResources.push_back(m_renderTarget.getUnorderedAccessView().getObject());
    frameResources.push_back(_targetView);
    m_frameExecutor.submitCommandLists(commandLists, std::move(frameResources));
    m_isPrepared = false;
}

void ego::render::DefaultRender::wait()
{
    m_frameExecutor.wait();
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

ego::gpu::Texture2DReference ego::render::DefaultRender::resolvePresentationTargetTexture(const gpu::TextureViewReference& _targetView) const
{
    if (!m_renderTarget.isReady() || !_targetView || _targetView->getViewType() != gpu::GraphicResourceViewType::RenderTarget ||
        _targetView->getDesc().m_dimension != gpu::TextureViewDimension::D2)
    {
        return nullptr;
    }

    const gpu::GraphicResourceReference& targetResource = _targetView->getResource();
    if (!targetResource || !rtti::IsObjectBasedOn<gpu::Texture2D>(*targetResource))
    {
        return nullptr;
    }

    const gpu::Texture2DReference targetTexture = targetResource.getObjectCast<gpu::Texture2D>();
    const gpu::Texture2DDesc& resultDesc = m_renderTarget.getTexture()->getDesc();
    const gpu::Texture2DDesc& targetDesc = targetTexture->getDesc();
    const gpu::GraphicResourceFormat targetViewFormat = _targetView->getDesc().m_format;
    if (targetDesc.m_size.m_x != resultDesc.m_size.m_x || targetDesc.m_size.m_y != resultDesc.m_size.m_y || targetDesc.m_format != resultDesc.m_format ||
        (targetViewFormat != gpu::GraphicResourceFormat::Undefined && targetViewFormat != targetDesc.m_format) || !(targetDesc.m_usage & gpu::TextureUsageRenderTarget))
    {
        return nullptr;
    }

    return targetTexture;
}

void ego::render::DefaultRender::copyResultToTarget(const RenderGraphicCommandList& _commandList, const gpu::Texture2DReference& _targetTexture)
{
    EGO_ASSERT(_commandList && m_renderTarget.isReady() && _targetTexture);

    m_renderTarget.transition(_commandList, gpu::GraphicResourceState::CopySrc);
    _commandList->resourceBarrier(_targetTexture, gpu::GraphicResourceState::CopyDst);

    const gpu::Texture2DSize& resultSize = m_renderTarget.getResolution();
    gpu::TextureCopyRegionDesc copyRegion;
    copyRegion.m_extent = UInt32Vector3(resultSize.m_x, resultSize.m_y, 1);
    _commandList->copyTexture(m_renderTarget.getTexture().getObject(), _targetTexture, copyRegion);
}

void ego::render::DefaultRender::handlePrepareFailure()
{
    m_scene.clear();
    m_passGraph.clearResources();
    m_pipelineStateCache.releaseUnused();
}
