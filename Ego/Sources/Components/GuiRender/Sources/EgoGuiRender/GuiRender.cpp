#include "GuiRender.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicDevice.h"

namespace
{
    struct GuiRootConstants final
    {
        ego::FloatVector2 m_viewportSize = ego::FloatVector2Zero;
        uint32_t m_textureIndex = ego::gpu::InvalidBindlessIndex;
        uint32_t m_samplerIndex = ego::gpu::InvalidBindlessIndex;
    };

    static_assert(sizeof(GuiRootConstants) == sizeof(uint32_t) * 4);
} // namespace

ego::gui::GuiRender::~GuiRender()
{
    release();
}

bool ego::gui::GuiRender::init(GraphicDevice& _graphicDevice, const InitData& _initData)
{
    release();

    EGO_CHECK_RETURN_FALSE(_graphicDevice.getCapabilities().m_supportsBindlessResources);
    EGO_CHECK_RETURN_FALSE(_initData.m_vertexShader && _initData.m_pixelShader);

    m_vertexShader = _initData.m_vertexShader;
    m_pixelShader = _initData.m_pixelShader;

    if (!initDefaultSampler(_graphicDevice) || !initBindingLayout(_graphicDevice))
    {
        release();
        return false;
    }

    m_isInitialized = true;
    return true;
}

void ego::gui::GuiRender::release()
{
    clearResources();
    m_pipelines.clear();
    m_bindingLayout = nullptr;
    m_defaultSampler = nullptr;
    m_pixelShader = nullptr;
    m_vertexShader = nullptr;
    m_isInitialized = false;
}

void ego::gui::GuiRender::clearResources()
{
    m_viewportResources.clear();
    m_imageResources.clear();
}

void ego::gui::GuiRender::removeViewport(ViewportID _viewportID)
{
    if (_viewportID != InvalidViewportID)
    {
        m_viewportResources.erase(_viewportID);
    }
}

bool ego::gui::GuiRender::prepare(GraphicDevice& _graphicDevice, GuiRenderPacket _packet)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(_packet.m_viewportID != InvalidViewportID);

    if (!_packet.m_drawData.isEmpty())
    {
        EGO_CHECK_RETURN_FALSE(_packet.m_drawData.m_viewportSize.m_x > 0.0f && _packet.m_drawData.m_viewportSize.m_y > 0.0f);
    }

    ViewportResources& resources = m_viewportResources[_packet.m_viewportID][_packet.m_frameIndex];
    EGO_CHECK_RETURN_FALSE(prepareImageBindings(_graphicDevice, _packet.m_imageBindings, resources));

    resources.m_drawData = std::move(_packet.m_drawData);
    return prepareBuffers(_graphicDevice, resources);
}

bool ego::gui::GuiRender::record(
    GraphicDevice& _graphicDevice,
    const gpu::GraphicCommandListReference& _commandList,
    const GuiRenderTarget& _target,
    ViewportID _viewportID,
    uint32_t _frameIndex)
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(_commandList);
    EGO_CHECK_RETURN_FALSE(_target.m_texture && _target.m_renderTargetView);
    EGO_CHECK_RETURN_FALSE(_target.m_renderTargetView->getViewType() == gpu::GraphicResourceViewType::RenderTarget);
    EGO_CHECK_RETURN_FALSE(_target.m_renderTargetView->getResource().getObject() == _target.m_texture.getObject());

    const ViewportResourceMap::const_iterator resourcesIt = m_viewportResources.find(_viewportID);
    EGO_CHECK_RETURN_FALSE(resourcesIt != m_viewportResources.end());

    const FrameResourceMap& frameResources = resourcesIt->second;
    const FrameResourceMap::const_iterator frameResourcesIt = frameResources.find(_frameIndex);
    EGO_CHECK_RETURN_FALSE(frameResourcesIt != frameResources.end());

    const ViewportResources& resources = frameResourcesIt->second;
    if (resources.m_drawData.isEmpty() && _target.m_loadOperation == gpu::AttachmentLoadOperation::Load)
    {
        return true;
    }

    const gpu::Texture2DDesc& targetDesc = _target.m_texture->getDesc();
    EGO_CHECK_RETURN_FALSE(targetDesc.m_size.m_x > 0 && targetDesc.m_size.m_y > 0);

    gpu::GraphicPipelineReference pipeline = nullptr;
    if (!resources.m_drawData.isEmpty())
    {
        const gpu::TextureViewDesc& targetViewDesc = _target.m_renderTargetView->getDesc();
        const gpu::GraphicResourceFormat targetFormat = targetViewDesc.m_format != gpu::GraphicResourceFormat::Undefined ? targetViewDesc.m_format : targetDesc.m_format;
        pipeline = getOrCreatePipeline(_graphicDevice, targetFormat, targetDesc.m_samples.m_count);
        EGO_CHECK_RETURN_FALSE(pipeline);

        transitionImageBindings(_commandList, resources);
    }

    if (_target.m_texture->getState() != gpu::GraphicResourceState::RenderTarget)
    {
        _commandList->resourceBarrier(_target.m_texture, gpu::GraphicResourceState::RenderTarget);
    }

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _target.m_renderTargetView;
    colorAttachment.m_loadOperation = _target.m_loadOperation;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = _target.m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = targetDesc.m_size;

    _commandList->beginRendering(renderingDesc);
    setupTargetViewport(_commandList, targetDesc.m_size);
    const bool renderResult = resources.m_drawData.isEmpty() || renderDrawData(_commandList, pipeline, targetDesc.m_size, resources);
    _commandList->endRendering();
    return renderResult;
}

bool ego::gui::GuiRender::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::GuiRender::initDefaultSampler(GraphicDevice& _graphicDevice)
{
    gpu::SamplerDesc samplerDesc;
    samplerDesc.m_minFilter = gpu::SamplerFilter::Linear;
    samplerDesc.m_magFilter = gpu::SamplerFilter::Linear;
    samplerDesc.m_mipFilter = gpu::SamplerFilter::Nearest;
    samplerDesc.m_addressU = gpu::SamplerAddressMode::ClampToEdge;
    samplerDesc.m_addressV = gpu::SamplerAddressMode::ClampToEdge;
    samplerDesc.m_addressW = gpu::SamplerAddressMode::ClampToEdge;

    m_defaultSampler = _graphicDevice.createSampler(samplerDesc);
    return m_defaultSampler && m_defaultSampler->getBindlessIndex() != gpu::InvalidBindlessIndex;
}

bool ego::gui::GuiRender::initBindingLayout(GraphicDevice& _graphicDevice)
{
    gpu::PushConstantRangeDesc guiConstants;
    guiConstants.m_offset = 0;
    guiConstants.m_size = sizeof(GuiRootConstants);
    guiConstants.m_stageFlag = gpu::ShaderStageFlagAll;

    gpu::BindingLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.m_pushConstants.push_back(guiConstants);

    m_bindingLayout = _graphicDevice.createBindingLayout(bindingLayoutDesc);
    return static_cast<bool>(m_bindingLayout);
}

ego::gpu::GraphicPipelineReference ego::gui::GuiRender::getOrCreatePipeline(GraphicDevice& _graphicDevice, gpu::GraphicResourceFormat _targetFormat, uint32_t _sampleCount)
{
    if (_targetFormat == gpu::GraphicResourceFormat::Undefined || _sampleCount == 0 || _sampleCount > static_cast<uint32_t>((std::numeric_limits<int>::max)()))
    {
        return nullptr;
    }

    for (const PipelineEntry& entry : m_pipelines)
    {
        if (entry.m_targetFormat == _targetFormat && entry.m_sampleCount == _sampleCount)
        {
            return entry.m_pipeline;
        }
    }

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
    pipelineDesc.m_depthFormat = gpu::GraphicResourceFormat::Undefined;
    pipelineDesc.m_multisampleCount = static_cast<int>(_sampleCount);
    pipelineDesc.m_colorFormats.push_back(_targetFormat);
    pipelineDesc.m_blendStateDesc.m_renderTargets.push_back(blendDesc);

    const gpu::GraphicPipelineReference pipeline = _graphicDevice.createGraphicPipeline(pipelineDesc);
    if (!pipeline)
    {
        return nullptr;
    }

    PipelineEntry entry;
    entry.m_targetFormat = _targetFormat;
    entry.m_sampleCount = _sampleCount;
    entry.m_pipeline = pipeline;
    m_pipelines.push_back(entry);
    return pipeline;
}

bool ego::gui::GuiRender::prepareImageBindings(GraphicDevice& _graphicDevice, const ImageBindingCollection& _imageBindings, ViewportResources& _resources)
{
    std::vector<PreparedImageBinding> preparedBindings;
    preparedBindings.reserve(_imageBindings.size());

    for (const ImageBinding& imageBinding : _imageBindings)
    {
        EGO_CHECK_RETURN_FALSE(imageBinding.m_id != InvalidImageID && imageBinding.m_image);
        EGO_CHECK_RETURN_FALSE(imageBinding.m_image->getID() == imageBinding.m_id);

        for (const PreparedImageBinding& preparedBinding : preparedBindings)
        {
            EGO_CHECK_RETURN_FALSE(preparedBinding.m_id != imageBinding.m_id);
        }

        gpu::Texture2DReference texture = nullptr;
        gpu::TextureViewReference textureView = nullptr;
        EGO_CHECK_RETURN_FALSE(getOrCreateImageResources(_graphicDevice, imageBinding.m_image, texture, textureView));

        PreparedImageBinding preparedBinding;
        preparedBinding.m_id = imageBinding.m_id;
        preparedBinding.m_texture = texture;
        preparedBinding.m_textureView = textureView;
        preparedBindings.push_back(preparedBinding);
    }

    _resources.m_imageBindings = std::move(preparedBindings);
    return true;
}

bool ego::gui::GuiRender::getOrCreateImageResources(
    GraphicDevice& _graphicDevice,
    const ImagePointer& _image,
    gpu::Texture2DReference& _texture,
    gpu::TextureViewReference& _textureView)
{
    for (const ImageResources& imageResources : m_imageResources)
    {
        if (imageResources.m_image.get() == _image.get())
        {
            _texture = imageResources.m_texture;
            _textureView = imageResources.m_textureView;
            return static_cast<bool>(_texture && _textureView);
        }
    }

    ImageResources imageResources;
    EGO_CHECK_RETURN_FALSE(createImageResources(_graphicDevice, _image, imageResources));

    _texture = imageResources.m_texture;
    _textureView = imageResources.m_textureView;
    m_imageResources.push_back(std::move(imageResources));
    return true;
}

bool ego::gui::GuiRender::createImageResources(GraphicDevice& _graphicDevice, const ImagePointer& _image, ImageResources& _resources)
{
    EGO_CHECK_RETURN_FALSE(_image);
    EGO_CHECK_RETURN_FALSE(_image->getFormat() == ImageFormat::R8);
    EGO_CHECK_RETURN_FALSE(_image->getWidth() > 0 && _image->getHeight() > 0);

    const Image::PixelCollection& pixels = _image->getPixels();
    const size_t expectedPixelCount = static_cast<size_t>(_image->getWidth()) * static_cast<size_t>(_image->getHeight());
    EGO_CHECK_RETURN_FALSE(pixels.size() == expectedPixelCount && pixels.size() <= static_cast<size_t>((std::numeric_limits<uint32_t>::max)()));

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GraphicResourceUsageTransferDst | gpu::GraphicResourceUsageShaderResource);
    textureDesc.m_size = gpu::Texture2DSize(_image->getWidth(), _image->getHeight());
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_format = gpu::GraphicResourceFormat::R8UNorm;

    const uint32_t pixelDataSize = static_cast<uint32_t>(pixels.size());
    const gpu::InitialGraphicResourceData initialData(pixels.data(), pixelDataSize, _image->getWidth(), pixelDataSize);

    gpu::GpuOperationOptions uploadOptions;
    uploadOptions.m_completionMode = gpu::GpuCompletionMode::WaitForCompletion;

    const gpu::GpuTexture2DTicket textureTicket = _graphicDevice.createAndUploadTexture2D(textureDesc, initialData, uploadOptions);
    EGO_CHECK_RETURN_FALSE(textureTicket.m_resource);
    textureTicket.waitReady();

    gpu::TextureViewDesc textureViewDesc;
    textureViewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
    textureViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    textureViewDesc.m_format = textureDesc.m_format;

    const gpu::TextureViewReference textureView = _graphicDevice.createTextureView(textureTicket.m_resource, textureViewDesc);
    EGO_CHECK_RETURN_FALSE(textureView && textureView->getBindlessIndex() != gpu::InvalidBindlessIndex);

    _resources.m_image = _image;
    _resources.m_texture = textureTicket.m_resource;
    _resources.m_textureView = textureView;
    return true;
}

bool ego::gui::GuiRender::prepareBuffers(GraphicDevice& _graphicDevice, ViewportResources& _resources)
{
    EGO_CHECK_RETURN_FALSE(prepareVertexBuffer(_graphicDevice, _resources));
    return prepareIndexBuffer(_graphicDevice, _resources);
}

bool ego::gui::GuiRender::prepareVertexBuffer(GraphicDevice& _graphicDevice, ViewportResources& _resources)
{
    const size_t vertexDataSize = _resources.m_drawData.m_vertices.size() * sizeof(Vertex);
    EGO_CHECK_RETURN_FALSE(vertexDataSize <= (std::numeric_limits<uint32_t>::max)());
    if (vertexDataSize == 0)
    {
        return true;
    }

    const uint32_t requiredBufferSize = static_cast<uint32_t>(vertexDataSize);
    if (!_resources.m_vertexBuffer || _resources.m_vertexBufferSize < requiredBufferSize)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_size = requiredBufferSize;
        bufferDesc.m_stride = sizeof(Vertex);
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);

        _resources.m_vertexBuffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(_resources.m_vertexBuffer);
        _resources.m_vertexBufferSize = requiredBufferSize;
    }

    void* data = _resources.m_vertexBuffer->map(0, requiredBufferSize);
    EGO_CHECK_RETURN_FALSE(data);
    std::memcpy(data, _resources.m_drawData.m_vertices.data(), requiredBufferSize);
    _resources.m_vertexBuffer->unmap(0, requiredBufferSize);
    return true;
}

bool ego::gui::GuiRender::prepareIndexBuffer(GraphicDevice& _graphicDevice, ViewportResources& _resources)
{
    const size_t indexDataSize = _resources.m_drawData.m_indices.size() * sizeof(uint32_t);
    EGO_CHECK_RETURN_FALSE(indexDataSize <= (std::numeric_limits<uint32_t>::max)());
    if (indexDataSize == 0)
    {
        return true;
    }

    const uint32_t requiredBufferSize = static_cast<uint32_t>(indexDataSize);
    if (!_resources.m_indexBuffer || _resources.m_indexBufferSize < requiredBufferSize)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_size = requiredBufferSize;
        bufferDesc.m_stride = sizeof(uint32_t);
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageIndexBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);

        _resources.m_indexBuffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(_resources.m_indexBuffer);
        _resources.m_indexBufferSize = requiredBufferSize;
    }

    void* data = _resources.m_indexBuffer->map(0, requiredBufferSize);
    EGO_CHECK_RETURN_FALSE(data);
    std::memcpy(data, _resources.m_drawData.m_indices.data(), requiredBufferSize);
    _resources.m_indexBuffer->unmap(0, requiredBufferSize);
    return true;
}

void ego::gui::GuiRender::transitionImageBindings(const gpu::GraphicCommandListReference& _commandList, const ViewportResources& _resources) const
{
    for (const PreparedImageBinding& imageBinding : _resources.m_imageBindings)
    {
        if (imageBinding.m_texture && imageBinding.m_texture->getState() != gpu::GraphicResourceState::ShaderRead)
        {
            _commandList->resourceBarrier(imageBinding.m_texture, gpu::GraphicResourceState::ShaderRead);
        }
    }
}

void ego::gui::GuiRender::setupTargetViewport(const gpu::GraphicCommandListReference& _commandList, const gpu::Texture2DSize& _targetSize) const
{
    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = static_cast<float>(_targetSize.m_x);
    viewportDesc.m_height = static_cast<float>(_targetSize.m_y);
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    _commandList->setViewport(viewportDesc);
}

bool ego::gui::GuiRender::renderDrawData(
    const gpu::GraphicCommandListReference& _commandList,
    const gpu::GraphicPipelineReference& _pipeline,
    const gpu::Texture2DSize& _targetSize,
    const ViewportResources& _resources) const
{
    EGO_CHECK_RETURN_FALSE(_pipeline && _resources.m_vertexBuffer && _resources.m_indexBuffer);

    const Size& viewportSize = _resources.m_drawData.m_viewportSize;
    EGO_CHECK_RETURN_FALSE(viewportSize.m_x > 0.0f && viewportSize.m_y > 0.0f);

    const float framebufferScaleX = static_cast<float>(_targetSize.m_x) / viewportSize.m_x;
    const float framebufferScaleY = static_cast<float>(_targetSize.m_y) / viewportSize.m_y;

    _commandList->setPipeline(_pipeline);
    _commandList->setVertexBuffer(0, _resources.m_vertexBuffer, sizeof(Vertex), 0);
    _commandList->setIndexBuffer(_resources.m_indexBuffer, gpu::GraphicResourceFormat::R32UInt, 0);

    for (const DrawCommand& command : _resources.m_drawData.m_commands)
    {
        gpu::ScissorRectDesc scissorRect;
        scissorRect.m_left = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.getLeft() * framebufferScaleX));
        scissorRect.m_top = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.getTop() * framebufferScaleY));
        scissorRect.m_right = static_cast<int32_t>((std::min)(static_cast<float>(_targetSize.m_x), command.m_clipRect.getRight() * framebufferScaleX));
        scissorRect.m_bottom = static_cast<int32_t>((std::min)(static_cast<float>(_targetSize.m_y), command.m_clipRect.getBottom() * framebufferScaleY));
        if (scissorRect.m_right <= scissorRect.m_left || scissorRect.m_bottom <= scissorRect.m_top)
        {
            continue;
        }

        _commandList->setScissorRect(scissorRect);

        GuiRootConstants guiConstants;
        guiConstants.m_viewportSize = viewportSize;

        const uint32_t textureIndex = findImageBindlessIndex(_resources, command.m_imageID);
        if (textureIndex != gpu::InvalidBindlessIndex && m_defaultSampler)
        {
            guiConstants.m_textureIndex = textureIndex;
            guiConstants.m_samplerIndex = m_defaultSampler->getBindlessIndex();
        }

        _commandList->pushConstants(gpu::ShaderStageFlagAll, 0, sizeof(guiConstants), &guiConstants);
        _commandList->drawIndexed(command.m_indexCount, 1, command.m_firstIndex, command.m_vertexOffset);
    }

    return true;
}

uint32_t ego::gui::GuiRender::findImageBindlessIndex(const ViewportResources& _resources, ImageID _imageID) const
{
    if (_imageID == InvalidImageID)
    {
        return gpu::InvalidBindlessIndex;
    }

    for (const PreparedImageBinding& imageBinding : _resources.m_imageBindings)
    {
        if (imageBinding.m_id == _imageID)
        {
            return imageBinding.m_textureView ? imageBinding.m_textureView->getBindlessIndex() : gpu::InvalidBindlessIndex;
        }
    }

    return gpu::InvalidBindlessIndex;
}

ego::gpu::InputLayoutDesc ego::gui::GuiRender::CreateGuiInputLayout()
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
