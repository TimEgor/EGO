#include "DefaultGuiRender.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"
#include "EgoGraphicHardware/Resources/ShaderResource.h"

#include "EgoResource/ResourceController.h"
#include "EgoResource/ResourceSubsystem.h"

#include "DefaultGuiRenderConstants.h"

namespace
{
    struct GuiRootConstants final
    {
        ego::FloatVector2 m_viewportSize = ego::FloatVector2Zero;
        uint32_t m_textureIndex = ego::gpu::InvalidBindlessIndex;
        uint32_t m_samplerIndex = ego::gpu::InvalidBindlessIndex;
        uint32_t m_textureSamplingMode = static_cast<uint32_t>(ego::gui::TextureSamplingMode::Alpha);
    };

    static_assert(sizeof(GuiRootConstants) == sizeof(uint32_t) * 5);
} // namespace

ego::gui::default_gui_render::DefaultGuiRender::~DefaultGuiRender()
{
    release();
}

bool ego::gui::default_gui_render::DefaultGuiRender::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    GraphicDevice& graphicDevice = gpu::GetGraphicDevice();
    EGO_CHECK_INITIALIZATION(graphicDevice.getCapabilities().m_supportsBindlessResources);

    m_commandQueue = gpu::GetGraphicCommandQueue();
    EGO_CHECK_INITIALIZATION(m_commandQueue && m_commandQueue->getCommandType() == gpu::CommandType::Graphic);

    m_commandList = graphicDevice.createGraphicCommandList();
    EGO_CHECK_INITIALIZATION(m_commandList);

    m_frameFence = graphicDevice.createFence();
    EGO_CHECK_INITIALIZATION(m_frameFence);

    FileName assetsRootPath;
    EGO_CHECK_INITIALIZATION(m_fileSystems.loadAssetsRootPath(assetsRootPath));
    EGO_CHECK_INITIALIZATION(m_fileSystems.initAssetsFileSystem(assetsRootPath));

    ResourceController& resourceController = GetResourceSubsystem().getResourceController();
    EGO_CHECK_INITIALIZATION(initShaders(resourceController));

    EGO_CHECK_INITIALIZATION(initSamplers());
    EGO_CHECK_INITIALIZATION(initBindingLayout());
    EGO_CHECK_INITIALIZATION(initPipeline());

    m_isInitialized = true;

    return true;
}

void ego::gui::default_gui_render::DefaultGuiRender::release()
{
    clearResources();

    m_pipeline = nullptr;
    m_bindingLayout = nullptr;
    m_nearestSampler = nullptr;
    m_linearSampler = nullptr;
    m_pixelShader = nullptr;
    m_vertexShader = nullptr;

    m_fileSystems.release();

    m_frameFence = nullptr;
    m_commandList = nullptr;
    m_commandQueue = nullptr;

    m_frameFenceValue = 0;

    m_isInitialized = false;
}

void ego::gui::default_gui_render::DefaultGuiRender::clearResources()
{
    waitFrame();

    m_targetViews.clear();
    m_viewportResources.clear();
    m_textureViews.clear();
}

bool ego::gui::default_gui_render::DefaultGuiRender::prepare(GuiRenderData&& _renderData)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);

    clearResources();

    for (const gpu::TextureViewPointer& textureView : _renderData.m_resourceTextureViews)
    {
        EGO_CHECK_RETURN_FALSE(textureView && textureView->getViewType() == gpu::GraphicResourceViewType::ShaderResource);
        EGO_CHECK_RETURN_FALSE(textureView->getDesc().m_dimension == gpu::TextureViewDimension::D2);
        EGO_CHECK_RETURN_FALSE(textureView->getBindlessIndex() != gpu::InvalidBindlessIndex);

        const gpu::GraphicResourcePointer& textureResource = textureView->getResource();
        EGO_CHECK_RETURN_FALSE(textureResource && rtti::IsObjectBasedOn<gpu::Texture2D>(*textureResource));
    }

    m_textureViews = std::move(_renderData.m_resourceTextureViews);

    std::vector<ViewportResources> viewportResources;
    viewportResources.reserve(_renderData.m_viewports.size());
    for (ViewportRenderData& viewportRenderData : _renderData.m_viewports)
    {
        ViewportResources resources;
        resources.m_drawData = std::move(viewportRenderData.m_drawData);
        if (!resources.m_drawData.isEmpty())
        {
            EGO_CHECK_RETURN_FALSE(resources.m_drawData.m_viewportSize.m_x > 0.0f);
            EGO_CHECK_RETURN_FALSE(resources.m_drawData.m_viewportSize.m_y > 0.0f);
            EGO_CHECK_RETURN_FALSE(resources.m_drawData.m_framebufferScale.m_x > 0.0f);
            EGO_CHECK_RETURN_FALSE(resources.m_drawData.m_framebufferScale.m_y > 0.0f);
        }
        EGO_CHECK_RETURN_FALSE(prepareBuffers(resources));

        viewportResources.push_back(std::move(resources));
    }

    m_viewportResources = std::move(viewportResources);
    return true;
}

bool ego::gui::default_gui_render::DefaultGuiRender::render(const TargetCollection& _targetViews)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(m_commandQueue)
    EGO_CHECK_RETURN_FALSE(m_commandList);
    EGO_CHECK_RETURN_FALSE(m_frameFence);
    EGO_CHECK_RETURN_FALSE(_targetViews.size() == m_viewportResources.size());

    if (_targetViews.empty())
    {
        return true;
    }

    m_targetViews = _targetViews;

    m_commandList->begin();
    transitionTextureViews();

    bool renderResult = true;
    for (size_t targetIndex = 0; targetIndex < m_targetViews.size(); ++targetIndex)
    {
        const gpu::TextureViewPointer& targetView = m_targetViews[targetIndex];
        const gpu::Texture2DPointer targetTexture = resolveTargetTexture(targetView);
        if (!targetTexture)
        {
            renderResult = false;
            continue;
        }

        if (!renderViewport(targetView, targetTexture, m_viewportResources[targetIndex]))
        {
            renderResult = false;
        }
    }

    m_commandList->end();
    m_commandQueue->execute(m_commandList);
    signalFrameFence();

    return renderResult;
}

bool ego::gui::default_gui_render::DefaultGuiRender::initShaders(ResourceController& _resourceController)
{
    const gpu::VertexShaderResourcePointer vertexShaderResource = _resourceController.load<gpu::VertexShaderResource>(GuiVertexShaderPath);
    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());

    const gpu::PixelShaderResourcePointer pixelShaderResource = _resourceController.load<gpu::PixelShaderResource>(GuiPixelShaderPath);
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    m_vertexShader = vertexShaderResource->getVertexShader();
    EGO_CHECK_RETURN_FALSE(m_vertexShader);
    m_pixelShader = pixelShaderResource->getPixelShader();
    EGO_CHECK_RETURN_FALSE(m_pixelShader);

    return true;
}

bool ego::gui::default_gui_render::DefaultGuiRender::initSamplers()
{
    gpu::SamplerDesc linearSamplerDesc;
    linearSamplerDesc.m_minFilter = gpu::SamplerFilter::Linear;
    linearSamplerDesc.m_magFilter = gpu::SamplerFilter::Linear;
    linearSamplerDesc.m_mipFilter = gpu::SamplerFilter::Linear;
    linearSamplerDesc.m_addressU = gpu::SamplerAddressMode::ClampToEdge;
    linearSamplerDesc.m_addressV = gpu::SamplerAddressMode::ClampToEdge;
    linearSamplerDesc.m_addressW = gpu::SamplerAddressMode::ClampToEdge;

    m_linearSampler = gpu::GetGraphicDevice().createSampler(linearSamplerDesc);
    EGO_CHECK_RETURN_FALSE(m_linearSampler);
    EGO_CHECK_RETURN_FALSE(m_linearSampler->getBindlessIndex() != gpu::InvalidBindlessIndex);

    gpu::SamplerDesc nearestSamplerDesc = linearSamplerDesc;
    nearestSamplerDesc.m_minFilter = gpu::SamplerFilter::Nearest;
    nearestSamplerDesc.m_magFilter = gpu::SamplerFilter::Nearest;
    nearestSamplerDesc.m_mipFilter = gpu::SamplerFilter::Nearest;

    m_nearestSampler = gpu::GetGraphicDevice().createSampler(nearestSamplerDesc);
    EGO_CHECK_RETURN_FALSE(m_nearestSampler);
    EGO_CHECK_RETURN_FALSE(m_nearestSampler->getBindlessIndex() != gpu::InvalidBindlessIndex);

    return true;
}

bool ego::gui::default_gui_render::DefaultGuiRender::initBindingLayout()
{
    gpu::PushConstantRangeDesc guiConstants;
    guiConstants.m_offset = 0;
    guiConstants.m_size = sizeof(GuiRootConstants);
    guiConstants.m_stageFlag = gpu::ShaderStageFlagAll;

    gpu::BindingLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.m_pushConstants.push_back(guiConstants);

    m_bindingLayout = gpu::GetGraphicDevice().createBindingLayout(bindingLayoutDesc);
    return static_cast<bool>(m_bindingLayout);
}

bool ego::gui::default_gui_render::DefaultGuiRender::initPipeline()
{
    gpu::RenderTargetBlendDesc blendDesc;
    blendDesc.m_blendEnable = true;
    blendDesc.m_srcColorFactor = gpu::BlendFactor::SrcAlpha;
    blendDesc.m_dstColorFactor = gpu::BlendFactor::OneMinusSrcAlpha;
    blendDesc.m_srcAlphaFactor = gpu::BlendFactor::One;
    blendDesc.m_dstAlphaFactor = gpu::BlendFactor::OneMinusSrcAlpha;

    gpu::GraphicPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = m_bindingLayout;
    pipelineDesc.m_vertexShader = m_vertexShader;
    pipelineDesc.m_pixelShader = m_pixelShader;
    pipelineDesc.m_inputLayoutDesc = CreateGuiInputLayout();
    pipelineDesc.m_topology = gpu::PrimitiveTopology::TriangleList;
    pipelineDesc.m_rasterizationStateDesc.m_cullMode = gpu::RasterizationCullMode::None;
    pipelineDesc.m_depthStencilStateDesc.m_depthTestEnable = false;
    pipelineDesc.m_depthStencilStateDesc.m_depthWrite = false;
    pipelineDesc.m_colorFormats.push_back(DefaultGuiRenderTargetFormat);
    pipelineDesc.m_blendStateDesc.m_renderTargets.push_back(blendDesc);

    m_pipeline = gpu::GetGraphicDevice().createGraphicPipeline(pipelineDesc);
    return static_cast<bool>(m_pipeline);
}

bool ego::gui::default_gui_render::DefaultGuiRender::prepareBuffers(ViewportResources& _resources)
{
    const size_t vertexDataSize = _resources.m_drawData.m_vertices.size() * sizeof(Vertex);
    const size_t indexDataSize = _resources.m_drawData.m_indices.size() * sizeof(uint32_t);
    EGO_CHECK_RETURN_FALSE(vertexDataSize <= (std::numeric_limits<uint32_t>::max)() && indexDataSize <= (std::numeric_limits<uint32_t>::max)());

    const gpu::InitialGraphicResourceData vertexData(_resources.m_drawData.m_vertices.data(), static_cast<uint32_t>(vertexDataSize));
    const gpu::InitialGraphicResourceData indexData(_resources.m_drawData.m_indices.data(), static_cast<uint32_t>(indexDataSize));
    EGO_CHECK_RETURN_FALSE(
        prepareBuffer(vertexData, sizeof(Vertex), static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer), _resources.m_vertexBuffer));
    EGO_CHECK_RETURN_FALSE(
        prepareBuffer(indexData, sizeof(uint32_t), static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageIndexBuffer), _resources.m_indexBuffer));

    return true;
}

bool ego::gui::default_gui_render::DefaultGuiRender::prepareBuffer(
    const gpu::InitialGraphicResourceData& _initialData,
    uint32_t _stride,
    gpu::GraphicResourceUsage _usage,
    gpu::BufferPointer& _buffer)
{
    if (!_initialData.isValid())
    {
        _buffer = nullptr;
        return true;
    }

    gpu::BufferDesc bufferDesc;
    bufferDesc.m_size = _initialData.m_dataSize;
    bufferDesc.m_stride = _stride;
    bufferDesc.m_usage = _usage;
    bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);

    _buffer = gpu::GetGraphicDevice().createBuffer(bufferDesc);
    EGO_CHECK_RETURN_FALSE(_buffer);

    void* data = _buffer->map(0, _initialData.m_dataSize);
    EGO_CHECK_RETURN_FALSE(data);
    std::memcpy(data, _initialData.m_data, _initialData.m_dataSize);
    _buffer->unmap(0, _initialData.m_dataSize);
    return true;
}

bool ego::gui::default_gui_render::DefaultGuiRender::renderViewport(
    const gpu::TextureViewPointer& _targetView,
    const gpu::Texture2DPointer& _targetTexture,
    const ViewportResources& _resources)
{
    if (_resources.m_drawData.isEmpty())
    {
        return true;
    }

    const gpu::Texture2DDesc& targetDesc = _targetTexture->getDesc();
    EGO_CHECK_RETURN_FALSE(targetDesc.m_size.m_x > 0 && targetDesc.m_size.m_y > 0);

    const gpu::TextureViewDesc& targetViewDesc = _targetView->getDesc();
    const gpu::GraphicResourceFormat targetFormat =
        targetViewDesc.m_format == gpu::GraphicResourceFormat::Undefined ? targetDesc.m_format : targetViewDesc.m_format;
    EGO_CHECK_RETURN_FALSE(targetFormat == DefaultGuiRenderTargetFormat);

    if (_targetTexture->getState() != gpu::GraphicResourceState::RenderTarget)
    {
        m_commandList->resourceBarrier(_targetTexture, gpu::GraphicResourceState::RenderTarget);
    }

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _targetView;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = targetDesc.m_size;

    m_commandList->beginRendering(renderingDesc);

    const FloatVector2& viewportSize = _resources.m_drawData.m_viewportSize;
    const FloatVector2& framebufferScale = _resources.m_drawData.m_framebufferScale;

    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = viewportSize.m_x * framebufferScale.m_x;
    viewportDesc.m_height = viewportSize.m_y * framebufferScale.m_y;
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    m_commandList->setViewport(viewportDesc);

    const bool renderResult = renderDrawData(targetDesc.m_size, _resources);
    m_commandList->endRendering();
    return renderResult;
}

bool ego::gui::default_gui_render::DefaultGuiRender::renderDrawData(const gpu::Texture2DSize& _targetSize, const ViewportResources& _resources) const
{
    EGO_CHECK_RETURN_FALSE(m_pipeline && _resources.m_vertexBuffer && _resources.m_indexBuffer);

    const FloatVector2& viewportSize = _resources.m_drawData.m_viewportSize;
    EGO_CHECK_RETURN_FALSE(viewportSize.m_x > 0.0f && viewportSize.m_y > 0.0f);

    const FloatVector2& framebufferScale = _resources.m_drawData.m_framebufferScale;
    EGO_CHECK_RETURN_FALSE(framebufferScale.m_x > 0.0f && framebufferScale.m_y > 0.0f);

    m_commandList->setPipeline(m_pipeline);
    m_commandList->setVertexBuffer(0, _resources.m_vertexBuffer, sizeof(Vertex), 0);
    m_commandList->setIndexBuffer(_resources.m_indexBuffer, gpu::GraphicResourceFormat::R32UInt, 0);

    for (const DrawCommand& command : _resources.m_drawData.m_commands)
    {
        gpu::ScissorRectDesc scissorRect;
        scissorRect.m_left = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.m_x * framebufferScale.m_x));
        scissorRect.m_top = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.m_y * framebufferScale.m_y));
        scissorRect.m_right = static_cast<int32_t>((std::min)(static_cast<float>(_targetSize.m_x), command.m_clipRect.m_z * framebufferScale.m_x));
        scissorRect.m_bottom = static_cast<int32_t>((std::min)(static_cast<float>(_targetSize.m_y), command.m_clipRect.m_w * framebufferScale.m_y));

        if (scissorRect.m_right <= scissorRect.m_left || scissorRect.m_bottom <= scissorRect.m_top)
        {
            continue;
        }

        m_commandList->setScissorRect(scissorRect);

        GuiRootConstants guiConstants;
        guiConstants.m_viewportSize = viewportSize;

        const gpu::SamplerPointer& sampler = command.m_textureFilteringMode == TextureFilteringMode::Nearest ? m_nearestSampler : m_linearSampler;
        if (command.m_textureIndex != gpu::InvalidBindlessIndex && sampler)
        {
            guiConstants.m_textureIndex = command.m_textureIndex;
            guiConstants.m_samplerIndex = sampler->getBindlessIndex();
            guiConstants.m_textureSamplingMode = static_cast<uint32_t>(command.m_textureSamplingMode);
        }

        m_commandList->pushConstants(gpu::ShaderStageFlagAll, 0, sizeof(guiConstants), &guiConstants);
        m_commandList->drawIndexed(command.m_indexCount, 1, command.m_firstIndex, command.m_vertexOffset);
    }

    return true;
}

ego::gpu::Texture2DPointer ego::gui::default_gui_render::DefaultGuiRender::resolveTargetTexture(const gpu::TextureViewPointer& _targetView) const
{
    if (!_targetView || _targetView->getViewType() != gpu::GraphicResourceViewType::RenderTarget ||
        _targetView->getDesc().m_dimension != gpu::TextureViewDimension::D2)
    {
        return nullptr;
    }

    const gpu::GraphicResourcePointer& targetResource = _targetView->getResource();
    if (!targetResource || !rtti::IsObjectBasedOn<gpu::Texture2D>(*targetResource))
    {
        return nullptr;
    }

    return gpu::Texture2DPointer(targetResource.getObjectCast<gpu::Texture2D>());
}

void ego::gui::default_gui_render::DefaultGuiRender::transitionTextureViews() const
{
    for (const gpu::TextureViewPointer& textureView : m_textureViews)
    {
        const gpu::GraphicResourcePointer& textureResource = textureView->getResource();
        const gpu::Texture2DPointer texture(textureResource.getObjectCast<gpu::Texture2D>());
        if (texture->getState() != gpu::GraphicResourceState::ShaderRead)
        {
            m_commandList->resourceBarrier(texture, gpu::GraphicResourceState::ShaderRead);
        }
    }
}

void ego::gui::default_gui_render::DefaultGuiRender::waitFrame()
{
    if (m_frameFence && m_frameFenceValue != 0)
    {
        m_frameFence->waitValue(m_frameFenceValue);
    }
}

void ego::gui::default_gui_render::DefaultGuiRender::signalFrameFence()
{
    EGO_ASSERT(m_frameFence && m_commandQueue);

    ++m_frameFenceValue;
    m_commandQueue->signal(m_frameFence, m_frameFenceValue);
}

ego::gpu::InputLayoutDesc ego::gui::default_gui_render::DefaultGuiRender::CreateGuiInputLayout()
{
    gpu::InputLayoutDesc inputLayout;

    gpu::InputLayoutBindingDesc bindingDesc;
    bindingDesc.m_slot = 0;
    bindingDesc.m_stride = sizeof(Vertex);
    bindingDesc.m_type = gpu::InputLayoutBindingType::VertexBinding;
    inputLayout.m_bindings.push_back(bindingDesc);

    gpu::InputLayoutElementDesc positionDesc;
    positionDesc.m_semanticName = "POSITION";
    positionDesc.m_location = 0;
    positionDesc.m_slot = 0;
    positionDesc.m_offset = offsetof(Vertex, m_position);
    positionDesc.m_componentsCount = 2;
    positionDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(positionDesc);

    gpu::InputLayoutElementDesc uvDesc;
    uvDesc.m_semanticName = "TEXCOORD";
    uvDesc.m_location = 1;
    uvDesc.m_slot = 0;
    uvDesc.m_offset = offsetof(Vertex, m_uv);
    uvDesc.m_componentsCount = 2;
    uvDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(uvDesc);

    gpu::InputLayoutElementDesc colorDesc;
    colorDesc.m_semanticName = "COLOR";
    colorDesc.m_location = 2;
    colorDesc.m_slot = 0;
    colorDesc.m_offset = offsetof(Vertex, m_color);
    colorDesc.m_componentsCount = 4;
    colorDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(colorDesc);

    return inputLayout;
}
