#include "DefaultRender.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"

bool ego::DefaultRender::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();

    gpu::CommandQueueDesc queueDesc;
    queueDesc.m_type = gpu::CommandType::Graphic;
    queueDesc.m_supportsPresentation = true;

    m_commandQueue = graphicDevice.createCommandQueue(queueDesc);
    EGO_CHECK_RETURN_FALSE(m_commandQueue);

    m_commandList = graphicDevice.createGraphicCommandList();
    EGO_CHECK_CALL(m_commandList, release());
    EGO_CHECK_RETURN_FALSE(m_commandList);

    m_isInitialized = true;
    return m_isInitialized;
}

void ego::DefaultRender::release()
{
    if (!m_isInitialized)
    {
        return;
    }

    wait();

    m_renderTargetView = nullptr;
    m_renderTargetTexture = nullptr;
    m_commandList = nullptr;
    m_commandQueue = nullptr;
    m_isInitialized = false;
}

void ego::DefaultRender::render(GraphicPresenter& _presenter)
{
    EGO_ASSERT(m_isInitialized);

    if (!m_isInitialized)
    {
        return;
    }

    EGO_ASSERT(m_commandQueue);
    EGO_ASSERT(m_commandList);

    if (!m_commandQueue || !m_commandList)
    {
        return;
    }

    const gpu::Texture2DReference targetTexture = _presenter.getTargetTexture();
    if (!prepareRenderTarget(targetTexture))
    {
        return;
    }

    m_commandList->begin();
    m_commandList->resourceBarrier(targetTexture, gpu::GraphicResourceState::Present, gpu::GraphicResourceState::RenderTarget);

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = m_renderTargetView;
    colorAttachment.m_loadOperation = m_clearEnabled
        ? gpu::AttachmentLoadOperation::Clear
        : gpu::AttachmentLoadOperation::Load;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = targetTexture->getDesc().m_size;

    m_commandList->beginRendering(renderingDesc);
    setupTargetViewport(targetTexture);

    for (const DefaultRender::Item& item : m_renderItems)
    {
        renderItem(item);
    }

    m_commandList->endRendering();
    m_commandList->resourceBarrier(targetTexture, gpu::GraphicResourceState::RenderTarget, gpu::GraphicResourceState::Present);
    m_commandList->end();

    m_commandQueue->execute(m_commandList);
}

bool ego::DefaultRender::isInitialized() const
{
    return m_isInitialized;
}

void ego::DefaultRender::wait()
{
    if (m_commandQueue)
    {
        m_commandQueue->waitIdle();
    }
}

void ego::DefaultRender::present(GraphicPresenter& _presenter)
{
    _presenter.present();
}

void ego::DefaultRender::setClearColor(const FloatVector4& _clearColor)
{
    m_clearColor = _clearColor;
}

const ego::FloatVector4& ego::DefaultRender::getClearColor() const
{
    return m_clearColor;
}

void ego::DefaultRender::setClearEnabled(bool _enabled)
{
    m_clearEnabled = _enabled;
}

bool ego::DefaultRender::isClearEnabled() const
{
    return m_clearEnabled;
}

void ego::DefaultRender::addRenderItem(
    const MeshReference& _mesh,
    const MaterialReference& _material
)
{
    if (!_mesh || !_material)
    {
        return;
    }

    m_renderItems.push_back(DefaultRender::Item{_mesh, _material});
}

void ego::DefaultRender::clearRenderItems()
{
    m_renderItems.clear();
}

const std::vector<ego::DefaultRender::Item>& ego::DefaultRender::getRenderItems() const
{
    return m_renderItems;
}

bool ego::DefaultRender::prepareRenderTarget(const gpu::Texture2DReference& _targetTexture)
{
    if (!_targetTexture)
    {
        return false;
    }

    const gpu::Texture2DDesc& targetDesc = _targetTexture->getDesc();
    if (targetDesc.m_size.m_x == 0 || targetDesc.m_size.m_y == 0)
    {
        return false;
    }

    if (m_renderTargetTexture.getObject() == _targetTexture.getObject() && m_renderTargetView)
    {
        return true;
    }

    gpu::TextureViewDesc viewDesc;
    viewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    viewDesc.m_dimension = gpu::TextureViewDimension::D2;
    viewDesc.m_format = targetDesc.m_format;

    m_renderTargetTexture = _targetTexture;
    m_renderTargetView = engine::GetEngine().getGraphicDevice().createTextureView(_targetTexture, viewDesc);

    return static_cast<bool>(m_renderTargetView);
}

void ego::DefaultRender::setupTargetViewport(const gpu::Texture2DReference& _targetTexture)
{
    EGO_ASSERT(_targetTexture);
    if (!_targetTexture)
    {
        return;
    }

    const gpu::Texture2DSize& targetSize = _targetTexture->getDesc().m_size;

    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = static_cast<float>(targetSize.m_x);
    viewportDesc.m_height = static_cast<float>(targetSize.m_y);
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    m_commandList->setViewport(viewportDesc);

    gpu::ScissorRectDesc scissorRectDesc;
    scissorRectDesc.m_right = static_cast<int32_t>(targetSize.m_x);
    scissorRectDesc.m_bottom = static_cast<int32_t>(targetSize.m_y);
    m_commandList->setScissorRect(scissorRectDesc);
}

void ego::DefaultRender::renderItem(const DefaultRender::Item& _item)
{
    if (!_item.m_mesh || !_item.m_material)
    {
        return;
    }

    const gpu::GraphicPipelineReference& pipeline = _item.m_material->getPipeline();
    if (!pipeline)
    {
        return;
    }

    m_commandList->setPipeline(pipeline);

    const Material::ResourceViewCollection& resourceViews = _item.m_material->getResourceViews();
    for (uint32_t resourceViewIndex = 0; resourceViewIndex < resourceViews.size(); ++resourceViewIndex)
    {
        m_commandList->bindResourceView(resourceViewIndex, resourceViews[resourceViewIndex]);
    }

    const Material::SamplerCollection& samplers = _item.m_material->getSamplers();
    for (uint32_t samplerIndex = 0; samplerIndex < samplers.size(); ++samplerIndex)
    {
        m_commandList->bindSampler(samplerIndex, samplers[samplerIndex]);
    }

    const Mesh::VertexBufferBinding& vertexBuffer = _item.m_mesh->getVertexBuffer();
    if (vertexBuffer.m_buffer && vertexBuffer.m_stride != 0)
    {
        m_commandList->setVertexBuffer(0, vertexBuffer.m_buffer, vertexBuffer.m_stride, vertexBuffer.m_offset);
    }

    const Mesh::IndexBufferBinding& indexBuffer = _item.m_mesh->getIndexBuffer();
    if (indexBuffer.m_buffer && _item.m_mesh->getIndexCount() != 0)
    {
        m_commandList->setIndexBuffer(
            indexBuffer.m_buffer,
            indexBuffer.m_format,
            indexBuffer.m_offset
        );
        m_commandList->drawIndexed(_item.m_mesh->getIndexCount());
        return;
    }

    if (_item.m_mesh->getVertexCount() != 0)
    {
        m_commandList->draw(_item.m_mesh->getVertexCount());
    }
}
