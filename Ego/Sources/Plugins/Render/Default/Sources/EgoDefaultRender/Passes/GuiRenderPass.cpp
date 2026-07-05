#include "GuiRenderPass.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Resource/ResourceController.h"
#include "EgoRuntime/RuntimeContext.h"

#include "EgoGraphicHardware/Resources/ShaderResource.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Render/RenderResourceObject.h"

namespace
{
    constexpr auto GuiVertexShaderPath = "Shaders/GuiRender/GuiVS.hlsl";
    constexpr auto GuiPixelShaderPath = "Shaders/GuiRender/GuiPS.hlsl";

    struct GuiRootConstants final
    {
        ego::FloatVector2 m_viewportSize = ego::FloatVector2Zero;
        uint32_t m_textureIndex = ego::gpu::InvalidBindlessIndex;
        uint32_t m_samplerIndex = ego::gpu::InvalidBindlessIndex;
    };

    static_assert(sizeof(GuiRootConstants) == sizeof(uint32_t) * 4);
} // namespace

bool ego::render::GuiRenderPass::init(RenderPassInitContext& _context)
{
    EGO_CHECK_RETURN_FALSE(loadShaders());
    EGO_CHECK_RETURN_FALSE(initFontSampler(_context.m_graphicDevice));
    EGO_CHECK_RETURN_FALSE(initBindingLayout(_context.m_graphicDevice));
    return initPipeline(_context.m_graphicDevice, _context.m_renderTargetFormat);
}

void ego::render::GuiRenderPass::release()
{
    clearResources();
    m_indexBuffer = nullptr;
    m_vertexBuffer = nullptr;
    m_pipeline = nullptr;
    m_pixelShader = nullptr;
    m_vertexShader = nullptr;
    m_fontSampler = nullptr;
    m_bindingLayout = nullptr;
    m_indexBufferSize = 0;
    m_vertexBufferSize = 0;
}

void ego::render::GuiRenderPass::clearResources()
{
    m_fontTextureView = nullptr;
    m_drawData.clear();
}

void ego::render::GuiRenderPass::declare(RenderPassBuilder& _builder)
{
    _builder.writeTexture("DefaultRenderTarget", gpu::GraphicResourceState::RenderTarget, gpu::GraphicResourceViewType::RenderTarget);
}

bool ego::render::GuiRenderPass::prepare(RenderPassPrepareContext& _context)
{
    gui::GuiController& guiController = engine::GetEngine().getGuiController();
    const gui::GuiViewportPointer guiViewport = guiController.getViewport();
    if (!guiController.isInitialized() || !guiViewport)
    {
        clearResources();
        return true;
    }

    const gpu::Texture2DReference& fontTexture = guiController.getFontTexture();
    EGO_CHECK_RETURN_FALSE(fontTexture);
    EGO_CHECK_RETURN_FALSE(prepareFontTextureView(_context.m_graphicDevice, fontTexture));
    EGO_CHECK_RETURN_FALSE(guiController.buildDrawData(m_drawData));
    return prepareBuffers(_context.m_graphicDevice);
}

void ego::render::GuiRenderPass::execute(RenderPassExecuteContext& _context)
{
    if (m_drawData.isEmpty() || !_context.m_commandList || !_context.m_renderTarget.getRenderTargetView())
    {
        return;
    }

    _context.m_renderTarget.transition(_context.m_commandList, gpu::GraphicResourceState::RenderTarget);

    if (m_fontTextureView)
    {
        _context.m_commandList->resourceBarrier(m_fontTextureView->getResource(), gpu::GraphicResourceState::ShaderRead);
    }

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = _context.m_renderTarget.getRenderTargetView().getObject();
    colorAttachment.m_loadOperation = gpu::AttachmentLoadOperation::Load;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = _context.m_settings.m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = _context.m_renderTarget.getResolution();

    _context.m_commandList->beginRendering(renderingDesc);
    setupTargetViewport(_context);
    renderDrawData(_context);
    _context.m_commandList->endRendering();
}

bool ego::render::GuiRenderPass::loadShaders()
{
    if (m_vertexShader && m_pixelShader)
    {
        return true;
    }

    ResourceController& resourceController = context::GetRuntimeContext().getResourceController();

    const gpu::VertexShaderResourcePointer vertexShaderResource = resourceController.load<gpu::VertexShaderResource>(GuiVertexShaderPath);
    EGO_CHECK_RETURN_FALSE(vertexShaderResource && vertexShaderResource->isLoaded());

    const gpu::PixelShaderResourcePointer pixelShaderResource = resourceController.load<gpu::PixelShaderResource>(GuiPixelShaderPath);
    EGO_CHECK_RETURN_FALSE(pixelShaderResource && pixelShaderResource->isLoaded());

    m_vertexShader = CreateVertexShaderHandler(vertexShaderResource);
    m_pixelShader = CreatePixelShaderHandler(pixelShaderResource);

    return m_vertexShader && m_pixelShader;
}

bool ego::render::GuiRenderPass::initFontSampler(GraphicDevice& _graphicDevice)
{
    if (m_fontSampler)
    {
        return true;
    }

    gpu::SamplerDesc samplerDesc;
    samplerDesc.m_minFilter = gpu::SamplerFilter::Linear;
    samplerDesc.m_magFilter = gpu::SamplerFilter::Linear;
    samplerDesc.m_mipFilter = gpu::SamplerFilter::Nearest;
    samplerDesc.m_addressU = gpu::SamplerAddressMode::ClampToEdge;
    samplerDesc.m_addressV = gpu::SamplerAddressMode::ClampToEdge;
    samplerDesc.m_addressW = gpu::SamplerAddressMode::ClampToEdge;

    m_fontSampler = _graphicDevice.createSampler(samplerDesc);
    return m_fontSampler && m_fontSampler->getBindlessIndex() != gpu::InvalidBindlessIndex;
}

bool ego::render::GuiRenderPass::initBindingLayout(GraphicDevice& _graphicDevice)
{
    if (m_bindingLayout)
    {
        return true;
    }

    gpu::PushConstantRangeDesc guiConstants;
    guiConstants.m_offset = 0;
    guiConstants.m_size = sizeof(GuiRootConstants);
    guiConstants.m_stageFlag = gpu::ShaderStageFlagAll;

    gpu::BindingLayoutDesc bindingLayoutDesc;
    bindingLayoutDesc.m_pushConstants.push_back(guiConstants);

    m_bindingLayout = _graphicDevice.createBindingLayout(bindingLayoutDesc);
    return m_bindingLayout.getObject() != nullptr;
}

bool ego::render::GuiRenderPass::initPipeline(GraphicDevice& _graphicDevice, gpu::GraphicResourceFormat _targetFormat)
{
    if (m_pipeline)
    {
        return true;
    }

    gpu::RenderTargetBlendDesc blendDesc;
    blendDesc.m_blendEnable = true;
    blendDesc.m_srcColorFactor = gpu::BlendFactor::SrcAlpha;
    blendDesc.m_dstColorFactor = gpu::BlendFactor::OneMinusSrcAlpha;
    blendDesc.m_srcAlphaFactor = gpu::BlendFactor::One;
    blendDesc.m_dstAlphaFactor = gpu::BlendFactor::OneMinusSrcAlpha;

    gpu::GraphicPipelineDesc pipelineDesc;
    pipelineDesc.m_bindingLayout = m_bindingLayout.getObject();
    pipelineDesc.m_vertexShader = m_vertexShader.getObject();
    pipelineDesc.m_pixelShader = m_pixelShader.getObject();
    pipelineDesc.m_inputLayoutDesc = CreateGuiInputLayout();
    pipelineDesc.m_topology = gpu::PrimitiveTopology::TriangleList;
    pipelineDesc.m_rasterizationStateDesc.m_cullMode = gpu::RasterizationCullMode::None;
    pipelineDesc.m_depthStencilStateDesc.m_depthTestEnable = false;
    pipelineDesc.m_depthStencilStateDesc.m_depthWrite = false;
    pipelineDesc.m_depthFormat = gpu::GraphicResourceFormat::Undefined;
    pipelineDesc.m_colorFormats.push_back(_targetFormat);
    pipelineDesc.m_blendStateDesc.m_renderTargets.push_back(blendDesc);

    m_pipeline = _graphicDevice.createGraphicPipeline(pipelineDesc);
    return m_pipeline.getObject() != nullptr;
}

bool ego::render::GuiRenderPass::prepareFontTextureView(
    GraphicDevice& _graphicDevice,
    const gpu::Texture2DReference& _fontTexture)
{
    if (m_fontTextureView && m_fontTextureView->getResource() == _fontTexture)
    {
        return m_fontTextureView->getBindlessIndex() != gpu::InvalidBindlessIndex;
    }

    const gpu::Texture2DDesc& textureDesc = _fontTexture->getDesc();

    gpu::TextureViewDesc textureViewDesc;
    textureViewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
    textureViewDesc.m_dimension = gpu::TextureViewDimension::D2;
    textureViewDesc.m_format = textureDesc.m_format;

    m_fontTextureView = _graphicDevice.createTextureView(_fontTexture, textureViewDesc);
    return m_fontTextureView && m_fontTextureView->getBindlessIndex() != gpu::InvalidBindlessIndex;
}

bool ego::render::GuiRenderPass::prepareBuffers(GraphicDevice& _graphicDevice)
{
    EGO_CHECK_RETURN_FALSE(prepareVertexBuffer(_graphicDevice));
    return prepareIndexBuffer(_graphicDevice);
}

bool ego::render::GuiRenderPass::prepareVertexBuffer(GraphicDevice& _graphicDevice)
{
    const uint32_t vertexDataSize = static_cast<uint32_t>(m_drawData.m_vertices.size() * sizeof(gui::GuiVertex));
    if (vertexDataSize == 0)
    {
        return true;
    }

    if (!m_vertexBuffer || m_vertexBufferSize < vertexDataSize)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_size = vertexDataSize;
        bufferDesc.m_stride = sizeof(gui::GuiVertex);
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);

        m_vertexBuffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(m_vertexBuffer);
        m_vertexBufferSize = vertexDataSize;
    }

    void* data = m_vertexBuffer->map(0, vertexDataSize);
    EGO_CHECK_RETURN_FALSE(data);
    std::memcpy(data, m_drawData.m_vertices.data(), vertexDataSize);
    m_vertexBuffer->unmap(0, vertexDataSize);

    return true;
}

bool ego::render::GuiRenderPass::prepareIndexBuffer(GraphicDevice& _graphicDevice)
{
    const uint32_t indexDataSize = static_cast<uint32_t>(m_drawData.m_indices.size() * sizeof(uint32_t));
    if (indexDataSize == 0)
    {
        return true;
    }

    if (!m_indexBuffer || m_indexBufferSize < indexDataSize)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_size = indexDataSize;
        bufferDesc.m_stride = sizeof(uint32_t);
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageIndexBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);

        m_indexBuffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(m_indexBuffer);
        m_indexBufferSize = indexDataSize;
    }

    void* data = m_indexBuffer->map(0, indexDataSize);
    EGO_CHECK_RETURN_FALSE(data);
    std::memcpy(data, m_drawData.m_indices.data(), indexDataSize);
    m_indexBuffer->unmap(0, indexDataSize);

    return true;
}

void ego::render::GuiRenderPass::setupTargetViewport(RenderPassExecuteContext& _context) const
{
    const gpu::Texture2DSize& resolution = _context.m_renderTarget.getResolution();

    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = static_cast<float>(resolution.m_x);
    viewportDesc.m_height = static_cast<float>(resolution.m_y);
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    _context.m_commandList->setViewport(viewportDesc);
}

void ego::render::GuiRenderPass::renderDrawData(RenderPassExecuteContext& _context) const
{
    if (!m_pipeline || !m_vertexBuffer || !m_indexBuffer)
    {
        return;
    }

    const gpu::Texture2DSize& resolution = _context.m_renderTarget.getResolution();
    const gpu::SamplerReference fontSampler = m_fontSampler.getObject();

    _context.m_commandList->setPipeline(m_pipeline.getObject());
    _context.m_commandList->setVertexBuffer(0, m_vertexBuffer.getObject(), sizeof(gui::GuiVertex), 0);
    _context.m_commandList->setIndexBuffer(m_indexBuffer.getObject(), gpu::GraphicResourceFormat::R32UInt, 0);

    for (const gui::GuiDrawCommand& command : m_drawData.m_commands)
    {
        gpu::ScissorRectDesc scissorRect;
        scissorRect.m_left = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.getLeft()));
        scissorRect.m_top = static_cast<int32_t>((std::max)(0.0f, command.m_clipRect.getTop()));
        scissorRect.m_right = static_cast<int32_t>((std::min)(static_cast<float>(resolution.m_x), command.m_clipRect.getRight()));
        scissorRect.m_bottom = static_cast<int32_t>((std::min)(static_cast<float>(resolution.m_y), command.m_clipRect.getBottom()));
        if (scissorRect.m_right <= scissorRect.m_left || scissorRect.m_bottom <= scissorRect.m_top)
        {
            continue;
        }

        _context.m_commandList->setScissorRect(scissorRect);

        GuiRootConstants guiConstants;
        guiConstants.m_viewportSize = FloatVector2(static_cast<float>(resolution.m_x), static_cast<float>(resolution.m_y));
        if (command.m_textureId != gui::InvalidGuiTextureID && m_fontTextureView && fontSampler)
        {
            guiConstants.m_textureIndex = m_fontTextureView->getBindlessIndex();
            guiConstants.m_samplerIndex = fontSampler->getBindlessIndex();
        }
        _context.m_commandList->pushConstants(gpu::ShaderStageFlagAll, 0, sizeof(guiConstants), &guiConstants);
        _context.m_commandList->drawIndexed(command.m_indexCount, 1, command.m_firstIndex, command.m_vertexOffset);
    }
}

ego::gpu::InputLayoutDesc ego::render::GuiRenderPass::CreateGuiInputLayout()
{
    gpu::InputLayoutDesc inputLayout;

    gpu::InputLayoutBindingDesc bindingDesc;
    bindingDesc.m_slot = 0;
    bindingDesc.m_stride = sizeof(gui::GuiVertex);
    bindingDesc.m_type = gpu::InputLayoutBindingType::VertexBinding;
    inputLayout.m_bindings.push_back(bindingDesc);

    gpu::InputLayoutElementDesc positionDesc;
    positionDesc.m_semanticName = "POSITION";
    positionDesc.m_location = 0;
    positionDesc.m_slot = 0;
    positionDesc.m_offset = offsetof(gui::GuiVertex, m_position);
    positionDesc.m_componentsCount = 2;
    positionDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(positionDesc);

    gpu::InputLayoutElementDesc uvDesc;
    uvDesc.m_semanticName = "TEXCOORD";
    uvDesc.m_location = 1;
    uvDesc.m_slot = 0;
    uvDesc.m_offset = offsetof(gui::GuiVertex, m_uv);
    uvDesc.m_componentsCount = 2;
    uvDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(uvDesc);

    gpu::InputLayoutElementDesc colorDesc;
    colorDesc.m_semanticName = "COLOR";
    colorDesc.m_location = 2;
    colorDesc.m_slot = 0;
    colorDesc.m_offset = offsetof(gui::GuiVertex, m_color);
    colorDesc.m_componentsCount = 4;
    colorDesc.m_type = gpu::InputLayoutElementType::Float32;
    inputLayout.m_elements.push_back(colorDesc);

    return inputLayout;
}
