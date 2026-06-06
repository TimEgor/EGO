#include "DefaultRenderDebugDraw.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/RenderHardware/Resources/ShaderResource.h"
#include "EgoEngine/Resources/GeneralResources/XmlResource.h"
#include "EgoEngine/Resources/Resource/ResourceController.h"

#include "RenderShaderData.h"

namespace
{
    constexpr uint32_t DebugDrawVertexStride = sizeof(ego::render::DebugDrawPointData);
    constexpr float DebugDrawPointHalfSize = 0.015f;
    constexpr const char* DefaultRenderConfigPath = "DefaultRender.xml";

    ego::FileName BuildPluginPath(const ego::FileName& _pluginRootPath, const char* _relativePath)
    {
        return _pluginRootPath && _relativePath ? _pluginRootPath + "/" + _relativePath : ego::FileName();
    }

    ego::FileName BuildPluginPath(const ego::FileName& _pluginRootPath, const ego::FileName& _relativePath)
    {
        return _pluginRootPath && _relativePath ? _pluginRootPath + "/" + _relativePath : ego::FileName();
    }

    struct DebugDrawConfig final
    {
        ego::FileName m_vertexShaderPath;
        ego::FileName m_pixelShaderPath;
    };

    bool ReadDebugDrawConfig(
        ego::ResourceController& _resourceController,
        const ego::FileName& _pluginRootPath,
        DebugDrawConfig& _config
    )
    {
        const ego::FileName configPath = BuildPluginPath(_pluginRootPath, DefaultRenderConfigPath);
        EGO_CHECK_RETURN_FALSE(configPath);

        const ego::XmlResourcePointer configResource = _resourceController.load<ego::XmlResource>(configPath);
        EGO_CHECK_RETURN_FALSE(configResource && configResource->isLoaded());

        const ego::XmlNode configNode = configResource->getRootNode();
        EGO_CHECK_RETURN_FALSE(configNode && configNode.getNameView() == "DefaultRender");

        const ego::XmlNode debugDrawNode = configNode.getChild("DebugDraw");
        EGO_CHECK_RETURN_FALSE(debugDrawNode);

        const std::string vertexShaderPath = debugDrawNode.getChildValueOr<std::string>(
            "VertexShader",
            std::string()
        );
        const std::string pixelShaderPath = debugDrawNode.getChildValueOr<std::string>(
            "PixelShader",
            std::string()
        );
        EGO_CHECK_RETURN_FALSE(!vertexShaderPath.empty() && !pixelShaderPath.empty());

        _config.m_vertexShaderPath = BuildPluginPath(_pluginRootPath, ego::FileName(vertexShaderPath));
        _config.m_pixelShaderPath = BuildPluginPath(_pluginRootPath, ego::FileName(pixelShaderPath));
        return _config.m_vertexShaderPath && _config.m_pixelShaderPath;
    }

    ego::gpu::InputLayoutDesc CreateDebugDrawInputLayout()
    {
        ego::gpu::InputLayoutDesc inputLayout;

        ego::gpu::InputLayoutBindingDesc bindingDesc;
        bindingDesc.m_slot = 0;
        bindingDesc.m_stride = DebugDrawVertexStride;
        bindingDesc.m_type = ego::gpu::InputLayoutBindingType::VertexBinding;
        inputLayout.m_bindings.push_back(bindingDesc);

        ego::gpu::InputLayoutElementDesc positionDesc;
        positionDesc.m_semanticName = "POSITION";
        positionDesc.m_location = 0;
        positionDesc.m_index = 0;
        positionDesc.m_slot = 0;
        positionDesc.m_offset = 0;
        positionDesc.m_componentsCount = 3;
        positionDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
        inputLayout.m_elements.push_back(positionDesc);

        ego::gpu::InputLayoutElementDesc colorDesc;
        colorDesc.m_semanticName = "COLOR";
        colorDesc.m_location = 1;
        colorDesc.m_index = 0;
        colorDesc.m_slot = 0;
        colorDesc.m_offset = sizeof(ego::FloatVector3);
        colorDesc.m_componentsCount = 4;
        colorDesc.m_type = ego::gpu::InputLayoutElementType::Float32;
        inputLayout.m_elements.push_back(colorDesc);

        return inputLayout;
    }

    ego::render::RenderGraphicPipeline CreateDebugDrawPipeline(
        ego::GraphicDevice& _graphicDevice,
        const ego::render::RenderBindingLayout& _bindingLayout,
        const ego::render::RenderVertexShader& _vertexShader,
        const ego::render::RenderPixelShader& _pixelShader,
        ego::gpu::GraphicResourceFormat _renderTargetFormat,
        ego::gpu::PrimitiveTopology _topology
    )
    {
        const ego::gpu::VertexShaderReference vertexShader = _vertexShader.getObject();
        const ego::gpu::PixelShaderReference pixelShader = _pixelShader.getObject();
        if (!vertexShader || !pixelShader)
        {
            return nullptr;
        }

        ego::gpu::GraphicPipelineDesc pipelineDesc;
        pipelineDesc.m_bindingLayout = _bindingLayout.getObject();
        pipelineDesc.m_vertexShader = vertexShader;
        pipelineDesc.m_pixelShader = pixelShader;
        pipelineDesc.m_inputLayoutDesc = CreateDebugDrawInputLayout();
        pipelineDesc.m_topology = _topology;
        pipelineDesc.m_rasterizationStateDesc.m_cullMode = ego::gpu::RasterizationCullMode::None;
        pipelineDesc.m_depthStencilStateDesc.m_depthTestEnable = false;
        pipelineDesc.m_depthStencilStateDesc.m_depthWrite = false;
        pipelineDesc.m_depthFormat = ego::gpu::GraphicResourceFormat::Undefined;
        pipelineDesc.m_colorFormats.push_back(_renderTargetFormat);

        return _graphicDevice.createGraphicPipeline(pipelineDesc);
    }
}

bool ego::render::DefaultRenderDebugDraw::init(
    const FileName& _pluginRootPath,
    GraphicDevice& _graphicDevice,
    const RenderBindingLayout& _bindingLayout,
    gpu::GraphicResourceFormat _renderTargetFormat
)
{
    ResourceController& resourceController = engine::GetEngine().getResourceController();

    DebugDrawConfig debugDrawConfig;
    EGO_CHECK_RETURN_FALSE(ReadDebugDrawConfig(resourceController, _pluginRootPath, debugDrawConfig));

    const gpu::VertexShaderResourcePointer vertexShaderResource =
        resourceController.load<gpu::VertexShaderResource>(debugDrawConfig.m_vertexShaderPath);
    const gpu::PixelShaderResourcePointer pixelShaderResource =
        resourceController.load<gpu::PixelShaderResource>(debugDrawConfig.m_pixelShaderPath);
    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    const RenderVertexShader vertexShader = CreateVertexShaderHandler(vertexShaderResource);
    const RenderPixelShader pixelShader = CreatePixelShaderHandler(pixelShaderResource);

    m_linePipeline = CreateDebugDrawPipeline(
        _graphicDevice,
        _bindingLayout,
        vertexShader,
        pixelShader,
        _renderTargetFormat,
        gpu::PrimitiveTopology::LineList
    );
    EGO_CHECK_RETURN_FALSE(m_linePipeline);

    return true;
}

void ego::render::DefaultRenderDebugDraw::release()
{
    clearResources();

    m_lineVertexBuffer = nullptr;
    m_lineVertexCapacity = 0;
    m_linePipeline = nullptr;
}

void ego::render::DefaultRenderDebugDraw::clearResources()
{
    clearCommands();
    m_lineVertexCount = 0;
}

bool ego::render::DefaultRenderDebugDraw::prepare()
{
    if (!prepareBuffer(m_lineVertices, m_lineVertexBuffer, m_lineVertexCapacity))
    {
        m_lineVertexCount = 0;
        return false;
    }

    m_lineVertexCount = static_cast<uint32_t>(m_lineVertices.size());
    clearCommands();
    return true;
}

void ego::render::DefaultRenderDebugDraw::render(
    const RenderGraphicCommandList& _commandList,
    const RenderBufferView& _cameraShaderDataView
)
{
    renderVertices(
        _commandList,
        _cameraShaderDataView,
        m_linePipeline,
        m_lineVertexBuffer,
        m_lineVertexCount
    );
    m_lineVertexCount = 0;
}

void ego::render::DefaultRenderDebugDraw::drawPoint(const DebugDrawPointData& _point)
{
    appendLineVertex(
        FloatVector3(
            _point.m_position.m_x - DebugDrawPointHalfSize,
            _point.m_position.m_y,
            _point.m_position.m_z
        ),
        _point.m_color
    );
    appendLineVertex(
        FloatVector3(
            _point.m_position.m_x + DebugDrawPointHalfSize,
            _point.m_position.m_y,
            _point.m_position.m_z
        ),
        _point.m_color
    );
    appendLineVertex(
        FloatVector3(
            _point.m_position.m_x,
            _point.m_position.m_y - DebugDrawPointHalfSize,
            _point.m_position.m_z
        ),
        _point.m_color
    );
    appendLineVertex(
        FloatVector3(
            _point.m_position.m_x,
            _point.m_position.m_y + DebugDrawPointHalfSize,
            _point.m_position.m_z
        ),
        _point.m_color
    );
}

void ego::render::DefaultRenderDebugDraw::drawLine(const DebugDrawLineData& _line)
{
    m_lineVertices.push_back(_line.m_start);
    m_lineVertices.push_back(_line.m_end);
}

void ego::render::DefaultRenderDebugDraw::clearCommands()
{
    m_lineVertices.clear();
}

void ego::render::DefaultRenderDebugDraw::appendLineVertex(
    const FloatVector3& _position,
    const FloatVector4& _color
)
{
    DebugDrawPointData vertex;
    vertex.m_position = _position;
    vertex.m_color = _color;
    m_lineVertices.push_back(vertex);
}

bool ego::render::DefaultRenderDebugDraw::prepareBuffer(
    const VertexCollection& _vertices,
    RenderBuffer& _buffer,
    uint32_t& _vertexCapacity
)
{
    if (_vertices.empty())
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(_vertices.size() <= (std::numeric_limits<uint32_t>::max)() / DebugDrawVertexStride);

    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();
    const uint32_t requiredCapacity = static_cast<uint32_t>(_vertices.size());

    if (!_buffer || _vertexCapacity < requiredCapacity)
    {
        const uint32_t newCapacity = (std::max)(requiredCapacity, (std::max)(_vertexCapacity * 2, 1u));

        gpu::BufferDesc bufferDesc;
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(
            gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead
        );
        bufferDesc.m_size = DebugDrawVertexStride * newCapacity;
        bufferDesc.m_stride = DebugDrawVertexStride;

        const RenderBuffer buffer = graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(buffer);

        _buffer = buffer;
        _vertexCapacity = newCapacity;
    }

    const uint32_t dataSize = DebugDrawVertexStride * requiredCapacity;
    void* mappedData = (*_buffer).map(0, dataSize);
    EGO_CHECK_RETURN_FALSE(mappedData);

    std::memcpy(mappedData, _vertices.data(), dataSize);
    (*_buffer).unmap(0, dataSize);
    return true;
}

void ego::render::DefaultRenderDebugDraw::renderVertices(
    const RenderGraphicCommandList& _commandList,
    const RenderBufferView& _cameraShaderDataView,
    const RenderGraphicPipeline& _pipeline,
    const RenderBuffer& _vertexBuffer,
    uint32_t _vertexCount
)
{
    if (!_commandList || !_pipeline || !_vertexBuffer || _vertexCount == 0 || !_cameraShaderDataView)
    {
        return;
    }

    RenderBindlessRootConstants rootConstants;
    rootConstants.m_cameraDataIndex = (*_cameraShaderDataView).getBindlessIndex();
    if (rootConstants.m_cameraDataIndex == gpu::InvalidBindlessIndex)
    {
        return;
    }

    (*_commandList).setPipeline(_pipeline.getObject());
    (*_commandList).pushConstants(
        RenderBindlessRootConstantsStageFlag,
        RenderBindlessRootConstantsOffset,
        sizeof(rootConstants),
        &rootConstants
    );
    (*_commandList).setVertexBuffer(0, _vertexBuffer.getObject(), DebugDrawVertexStride, 0);
    (*_commandList).draw(_vertexCount);
}
