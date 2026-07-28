#include "DebugRender.h"

#include <algorithm>
#include <cstddef>
#include <limits>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoDefaultRender/PipelineState/RenderPipelineStateCache.h"
#include "EgoDefaultRender/RenderShaderData.h"

namespace
{
    constexpr uint32_t DebugPointDataStride = sizeof(ego::render::DebugPointRenderData::PointData);
    constexpr uint32_t DebugPointCountPerBuffer = 512;
    constexpr uint32_t DebugLineVertexStride = sizeof(ego::render::DebugLineRenderData::VertexData);
    constexpr uint32_t DebugLineCountPerBuffer = 512;
} // namespace

ego::gpu::InputLayoutDesc ego::render::DefaultRenderDebugDraw::CreatePointInputLayout()
{
    ego::gpu::InputLayoutDesc inputLayout;

    ego::gpu::InputLayoutBindingDesc bindingDesc;
    bindingDesc.m_slot = 0;
    bindingDesc.m_stride = DebugPointDataStride;
    bindingDesc.m_type = ego::gpu::InputLayoutBindingType::InstanceBinding;
    inputLayout.m_bindings.push_back(bindingDesc);

    ego::gpu::InputLayoutElementDesc positionDesc;
    positionDesc.m_semanticName = "INST_POSITION";
    positionDesc.m_location = 0;
    positionDesc.m_index = 0;
    positionDesc.m_slot = 0;
    positionDesc.m_offset = offsetof(ego::render::DebugPointRenderData::PointData, m_position);
    positionDesc.m_componentsCount = 3;
    positionDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(positionDesc);

    ego::gpu::InputLayoutElementDesc colorDesc;
    colorDesc.m_semanticName = "INST_COLOR";
    colorDesc.m_location = 1;
    colorDesc.m_index = 0;
    colorDesc.m_slot = 0;
    colorDesc.m_offset = offsetof(ego::render::DebugPointRenderData::PointData, m_color);
    colorDesc.m_componentsCount = 3;
    colorDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(colorDesc);

    return inputLayout;
}

ego::gpu::InputLayoutDesc ego::render::DefaultRenderDebugDraw::CreateLineInputLayout()
{
    ego::gpu::InputLayoutDesc inputLayout;

    ego::gpu::InputLayoutBindingDesc bindingDesc;
    bindingDesc.m_slot = 0;
    bindingDesc.m_stride = DebugLineVertexStride;
    bindingDesc.m_type = ego::gpu::InputLayoutBindingType::VertexBinding;
    inputLayout.m_bindings.push_back(bindingDesc);

    ego::gpu::InputLayoutElementDesc positionDesc;
    positionDesc.m_semanticName = "POSITION";
    positionDesc.m_location = 0;
    positionDesc.m_index = 0;
    positionDesc.m_slot = 0;
    positionDesc.m_offset = offsetof(ego::render::DebugLineRenderData::VertexData, m_position);
    positionDesc.m_componentsCount = 3;
    positionDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(positionDesc);

    ego::gpu::InputLayoutElementDesc colorDesc;
    colorDesc.m_semanticName = "COLOR";
    colorDesc.m_location = 1;
    colorDesc.m_index = 0;
    colorDesc.m_slot = 0;
    colorDesc.m_offset = offsetof(ego::render::DebugLineRenderData::VertexData, m_color);
    colorDesc.m_componentsCount = 3;
    colorDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(colorDesc);

    return inputLayout;
}

ego::render::RasterizationMaterialRenderPassInfoPointer ego::render::DefaultRenderDebugDraw::CreateDebugDrawMaterialInfo(
    const ego::render::RenderVertexShader& _vertexShader,
    const ego::render::RenderPixelShader& _pixelShader)
{
    const ego::gpu::VertexShaderPointer vertexShader = _vertexShader.getObject();
    const ego::gpu::PixelShaderPointer pixelShader = _pixelShader.getObject();
    if (!vertexShader || !pixelShader)
    {
        return nullptr;
    }

    return MakeIntrusive<RasterizationMaterialRenderPassInfo>(_vertexShader, _pixelShader);
}

ego::render::RenderGraphicPipeline ego::render::DefaultRenderDebugDraw::getOrCreateDebugDrawPipeline(
    ego::GraphicDevice& _graphicDevice,
    ego::render::RenderPipelineStateCache& _pipelineStateCache,
    const RasterizationMaterialRenderPassInfoPointer& _materialInfo,
    const ego::gpu::InputLayoutDesc& _inputLayoutDesc,
    ego::gpu::PrimitiveTopology _topology) const
{
    if (!_materialInfo || !m_bindingLayout)
    {
        return nullptr;
    }

    ego::gpu::GraphicPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = m_bindingLayout.getObject();
    pipelineDesc.m_vertexShader = _materialInfo->getVertexShader().getObject();
    pipelineDesc.m_pixelShader = _materialInfo->getPixelShader().getObject();
    pipelineDesc.m_inputLayoutDesc = _inputLayoutDesc;
    pipelineDesc.m_topology = _topology;
    pipelineDesc.m_rasterizationStateDesc.m_cullMode = ego::gpu::RasterizationCullMode::None;
    pipelineDesc.m_depthStencilStateDesc.m_depthTestEnable = false;
    pipelineDesc.m_depthStencilStateDesc.m_depthWrite = false;
    pipelineDesc.m_depthFormat = ego::gpu::GraphicResourceFormat::Undefined;
    pipelineDesc.m_colorFormats.push_back(m_renderTargetFormat);

    return _pipelineStateCache.getOrCreateGraphicPipeline(_graphicDevice, pipelineDesc);
}

bool ego::render::DefaultRenderDebugDraw::init(
    const RenderBindingLayout& _bindingLayout,
    gpu::GraphicResourceFormat _renderTargetFormat,
    const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(_initData.m_point.m_vertexShader);
    EGO_CHECK_INITIALIZATION(_initData.m_point.m_pixelShader);
    EGO_CHECK_INITIALIZATION(_initData.m_line.m_vertexShader);
    EGO_CHECK_INITIALIZATION(_initData.m_line.m_pixelShader);

    m_bindingLayout = _bindingLayout;
    m_renderTargetFormat = _renderTargetFormat;

    m_pointRenderData.m_materialInfo = CreateDebugDrawMaterialInfo(_initData.m_point.m_vertexShader, _initData.m_point.m_pixelShader);
    EGO_CHECK_INITIALIZATION(m_pointRenderData.m_materialInfo);

    m_lineRenderData.m_materialInfo = CreateDebugDrawMaterialInfo(_initData.m_line.m_vertexShader, _initData.m_line.m_pixelShader);
    EGO_CHECK_INITIALIZATION(m_lineRenderData.m_materialInfo);

    DebugElementBufferPool::BufferDesc pointDataBufferDesc;
    pointDataBufferDesc.m_stride = sizeof(DebugPointRenderData::PointData);
    pointDataBufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
    pointDataBufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);
    EGO_CHECK_INITIALIZATION(m_pointRenderData.m_pointData.init(pointDataBufferDesc, DebugPointCountPerBuffer));

    DebugElementBufferPool::BufferDesc lineDataBufferDesc;
    lineDataBufferDesc.m_stride = sizeof(DebugLineRenderData::LineData);
    lineDataBufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
    lineDataBufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);
    EGO_CHECK_INITIALIZATION(m_lineRenderData.m_lineData.init(lineDataBufferDesc, DebugLineCountPerBuffer));

    return true;
}

void ego::render::DefaultRenderDebugDraw::release()
{
    clearResources();

    m_bindingLayout = nullptr;
    m_renderTargetFormat = gpu::GraphicResourceFormat::Undefined;

    m_pointRenderData.m_pointData.release();
    m_pointRenderData.m_materialInfo = nullptr;

    m_lineRenderData.m_lineData.release();
    m_lineRenderData.m_materialInfo = nullptr;
}

void ego::render::DefaultRenderDebugDraw::clearResources()
{
    clearCommands();

    m_pointRenderData.m_pointCount = 0;
    m_lineRenderData.m_lineCount = 0;
}

bool ego::render::DefaultRenderDebugDraw::prepare(float _deltaTime)
{
    return preparePointData(_deltaTime) && prepareLineData(_deltaTime);
}

void ego::render::DefaultRenderDebugDraw::render(
    GraphicDevice& _graphicDevice,
    RenderPipelineStateCache& _pipelineStateCache,
    const RenderGraphicCommandList& _commandList,
    const RenderBufferView& _cameraShaderDataView)
{
    renderPoints(_graphicDevice, _pipelineStateCache, _commandList, _cameraShaderDataView);
    renderLines(_graphicDevice, _pipelineStateCache, _commandList, _cameraShaderDataView);

    m_pointRenderData.m_pointCount = 0;
    m_lineRenderData.m_lineCount = 0;
}

void ego::render::DefaultRenderDebugDraw::drawPoint(const DebugDrawPointData& _point)
{
    m_pointRenderData.m_pointCommands.emplace_back(_point.m_position, _point.m_color);
}

void ego::render::DefaultRenderDebugDraw::drawLine(const DebugDrawLineData& _line)
{
    appendLine(
        DebugLineRenderData::VertexData(_line.m_start.m_position, _line.m_start.m_color),
        DebugLineRenderData::VertexData(_line.m_end.m_position, _line.m_end.m_color));
}

void ego::render::DefaultRenderDebugDraw::clearCommands()
{
    m_pointRenderData.m_pointCommands.clear();
    m_lineRenderData.m_lineCommands.clear();
}

void ego::render::DefaultRenderDebugDraw::appendLine(const DebugLineRenderData::VertexData& _start, const DebugLineRenderData::VertexData& _end)
{
    m_lineRenderData.m_lineCommands.emplace_back(_start, _end);
}

bool ego::render::DefaultRenderDebugDraw::preparePointData(float _deltaTime)
{
    EGO_CHECK_RETURN_FALSE(m_pointRenderData.m_pointCommands.size() <= (std::numeric_limits<uint32_t>::max)());

    m_pointRenderData.m_pointCount = static_cast<uint32_t>(m_pointRenderData.m_pointCommands.size());
    EGO_CHECK_RETURN_FALSE(m_pointRenderData.m_pointData.updateBuffers(_deltaTime, m_pointRenderData.m_pointCount));

    if (m_pointRenderData.m_pointCount == 0)
    {
        m_pointRenderData.m_pointCommands.clear();
        return true;
    }

    DebugElementBufferPoolIterator pointDataIter(m_pointRenderData.m_pointData);
    for (uint32_t pointIndex = 0; pointIndex < m_pointRenderData.m_pointCount; ++pointIndex)
    {
        DebugPointRenderData::PointData* pointData = pointDataIter.getCurrentTypedElement<DebugPointRenderData::PointData>();
        EGO_CHECK_RETURN_FALSE(pointData);

        *pointData = m_pointRenderData.m_pointCommands[pointIndex];
        pointDataIter.next();
    }

    m_pointRenderData.m_pointCommands.clear();
    return true;
}

bool ego::render::DefaultRenderDebugDraw::prepareLineData(float _deltaTime)
{
    EGO_CHECK_RETURN_FALSE(m_lineRenderData.m_lineCommands.size() <= (std::numeric_limits<uint32_t>::max)());

    m_lineRenderData.m_lineCount = static_cast<uint32_t>(m_lineRenderData.m_lineCommands.size());
    EGO_CHECK_RETURN_FALSE(m_lineRenderData.m_lineData.updateBuffers(_deltaTime, m_lineRenderData.m_lineCount));

    if (m_lineRenderData.m_lineCount == 0)
    {
        clearCommands();
        return true;
    }

    DebugElementBufferPoolIterator lineDataIter(m_lineRenderData.m_lineData);
    for (uint32_t lineIndex = 0; lineIndex < m_lineRenderData.m_lineCount; ++lineIndex)
    {
        DebugLineRenderData::LineData* lineData = lineDataIter.getCurrentTypedElement<DebugLineRenderData::LineData>();
        EGO_CHECK_RETURN_FALSE(lineData);

        *lineData = m_lineRenderData.m_lineCommands[lineIndex];
        lineDataIter.next();
    }

    clearCommands();

    return true;
}

void ego::render::DefaultRenderDebugDraw::renderPoints(
    GraphicDevice& _graphicDevice,
    RenderPipelineStateCache& _pipelineStateCache,
    const RenderGraphicCommandList& _commandList,
    const RenderBufferView& _cameraShaderDataView)
{
    if (!_commandList || !m_pointRenderData.m_materialInfo || m_pointRenderData.m_pointCount == 0 || !_cameraShaderDataView)
    {
        return;
    }

    RenderBindlessRootConstants rootConstants;
    rootConstants.m_cameraDataIndex = _cameraShaderDataView->getBindlessIndex();
    if (rootConstants.m_cameraDataIndex == gpu::InvalidBindlessIndex)
    {
        return;
    }

    const RenderGraphicPipeline pipeline = getOrCreateDebugDrawPipeline(
        _graphicDevice,
        _pipelineStateCache,
        m_pointRenderData.m_materialInfo,
        CreatePointInputLayout(),
        gpu::PrimitiveTopology::TriangleStrip);
    if (!pipeline)
    {
        return;
    }

    _commandList->setPipeline(pipeline.getObject());
    _commandList->pushConstants(RenderBindlessRootConstantsStageFlag, RenderBindlessRootConstantsOffset, sizeof(rootConstants), &rootConstants);

    uint32_t remainedPointCount = m_pointRenderData.m_pointCount;
    const uint32_t pointCountPerBuffer = m_pointRenderData.m_pointData.getElementCountPerBuffer();
    const uint32_t bufferCount = m_pointRenderData.m_pointData.getBufferCount();
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount && remainedPointCount > 0; ++bufferIndex)
    {
        const uint32_t drawingPointCount = (std::min)(remainedPointCount, pointCountPerBuffer);
        const RenderBuffer& pointBuffer = m_pointRenderData.m_pointData.getBuffer(bufferIndex);
        if (!pointBuffer)
        {
            return;
        }

        _commandList->setVertexBuffer(0, pointBuffer.getObject(), DebugPointDataStride, 0);
        _commandList->draw(4, drawingPointCount);

        remainedPointCount -= drawingPointCount;
    }
}

void ego::render::DefaultRenderDebugDraw::renderLines(
    GraphicDevice& _graphicDevice,
    RenderPipelineStateCache& _pipelineStateCache,
    const RenderGraphicCommandList& _commandList,
    const RenderBufferView& _cameraShaderDataView)
{
    if (!_commandList || !m_lineRenderData.m_materialInfo || m_lineRenderData.m_lineCount == 0 || !_cameraShaderDataView)
    {
        return;
    }

    RenderBindlessRootConstants rootConstants;
    rootConstants.m_cameraDataIndex = _cameraShaderDataView->getBindlessIndex();
    if (rootConstants.m_cameraDataIndex == gpu::InvalidBindlessIndex)
    {
        return;
    }

    const RenderGraphicPipeline pipeline = getOrCreateDebugDrawPipeline(
        _graphicDevice,
        _pipelineStateCache,
        m_lineRenderData.m_materialInfo,
        CreateLineInputLayout(),
        gpu::PrimitiveTopology::LineList);
    if (!pipeline)
    {
        return;
    }

    _commandList->setPipeline(pipeline.getObject());
    _commandList->pushConstants(RenderBindlessRootConstantsStageFlag, RenderBindlessRootConstantsOffset, sizeof(rootConstants), &rootConstants);

    uint32_t remainedLineCount = m_lineRenderData.m_lineCount;
    const uint32_t lineCountPerBuffer = m_lineRenderData.m_lineData.getElementCountPerBuffer();
    const uint32_t bufferCount = m_lineRenderData.m_lineData.getBufferCount();
    for (uint32_t bufferIndex = 0; bufferIndex < bufferCount && remainedLineCount > 0; ++bufferIndex)
    {
        const uint32_t drawingLineCount = (std::min)(remainedLineCount, lineCountPerBuffer);
        const RenderBuffer& lineBuffer = m_lineRenderData.m_lineData.getBuffer(bufferIndex);
        if (!lineBuffer)
        {
            return;
        }

        _commandList->setVertexBuffer(0, lineBuffer.getObject(), DebugLineVertexStride, 0);
        _commandList->draw(drawingLineCount * 2);

        remainedLineCount -= drawingLineCount;
    }
}
