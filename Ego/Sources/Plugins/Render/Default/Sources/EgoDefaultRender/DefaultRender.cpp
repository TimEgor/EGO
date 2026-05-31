#include "DefaultRender.h"

#include <algorithm>
#include <cstring>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Memory/Utils.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"
#include "EgoEngine/Graphic/Render/RenderShaderData.h"
#include "EgoEngine/Level/Level.h"

namespace
{
    constexpr ego::gpu::GraphicResourceFormat RenderTargetFormat = ego::gpu::GraphicResourceFormat::B8G8R8A8UNorm;
    constexpr uint32_t ConstantBufferAlignment = 256;
}

bool ego::DefaultRender::init()
{
    if (m_isInitialized)
    {
        return true;
    }

    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();
    EGO_CHECK_RETURN_FALSE(graphicDevice.getCapabilities().m_supportsBindlessResources);

    gpu::CommandQueueDesc queueDesc;
    queueDesc.m_type = gpu::CommandType::Graphic;
    queueDesc.m_supportsPresentation = true;

    m_commandQueue = graphicDevice.createCommandQueue(queueDesc);
    EGO_CHECK_RETURN_FALSE(m_commandQueue);

    m_commandList = graphicDevice.createGraphicCommandList();
    EGO_CHECK_CALL(m_commandList, release());
    EGO_CHECK_RETURN_FALSE(m_commandList);

    m_presentCommandList = graphicDevice.createGraphicCommandList();
    EGO_CHECK_CALL(m_presentCommandList, release());
    EGO_CHECK_RETURN_FALSE(m_presentCommandList);

    m_frameFence = graphicDevice.createFence();
    EGO_CHECK_CALL(m_frameFence, release());
    EGO_CHECK_RETURN_FALSE(m_frameFence);

    m_isInitialized = true;
    return m_isInitialized;
}

void ego::DefaultRender::release()
{
    clearResources();

    m_objectShaderDataView = nullptr;
    m_objectShaderDataBuffer = nullptr;
    m_objectShaderDataCapacity = 0;
    m_cameraShaderDataView = nullptr;
    m_cameraShaderDataBuffer = nullptr;

    m_renderTargetView = nullptr;
    m_renderTargetTexture = nullptr;
    m_renderTargetState = gpu::GraphicResourceState::Common;
    m_isPrepared = false;

    m_commandList = nullptr;
    m_presentCommandList = nullptr;
    m_frameFence = nullptr;
    m_frameFenceValue = 0;
    m_commandQueue = nullptr;

    m_isInitialized = false;
}

void ego::DefaultRender::clearResources()
{
    wait();

    m_renderItems.clear();
    m_isPrepared = false;
}

bool ego::DefaultRender::prepare(Level& _level, ecs::Entity _cameraEntity)
{
    EGO_ASSERT(m_isInitialized);

    if (!m_isInitialized)
    {
        return false;
    }

    EGO_ASSERT(m_commandQueue);
    EGO_ASSERT(m_commandList);

    if (!m_commandQueue || !m_commandList)
    {
        return false;
    }

    m_isPrepared = false;

    if (!prepareRenderTarget())
    {
        return false;
    }

    collectRenderItems(_level);

    if (!prepareShaderData(_level, _cameraEntity))
    {
        m_renderItems.clear();
        return false;
    }

    m_isPrepared = true;
    return true;
}

void ego::DefaultRender::render()
{
    EGO_ASSERT(m_isInitialized);

    if (!m_isInitialized || !m_isPrepared)
    {
        return;
    }

    EGO_ASSERT(m_commandQueue);
    EGO_ASSERT(m_commandList);
    EGO_ASSERT(m_renderTargetTexture);
    EGO_ASSERT(m_renderTargetView);

    if (!m_commandQueue || !m_commandList || !m_renderTargetTexture || !m_renderTargetView)
    {
        return;
    }

    m_commandList->begin();
    transitionRenderTarget(m_commandList, gpu::GraphicResourceState::RenderTarget);

    gpu::ColorAttachmentDesc colorAttachment;
    colorAttachment.m_view = m_renderTargetView;
    colorAttachment.m_loadOperation = m_clearEnabled
        ? gpu::AttachmentLoadOperation::Clear
        : gpu::AttachmentLoadOperation::Load;
    colorAttachment.m_storeOperation = gpu::AttachmentStoreOperation::Store;
    colorAttachment.m_clearValue = m_clearColor;

    gpu::RenderingDesc renderingDesc;
    renderingDesc.m_colorAttachments.push_back(colorAttachment);
    renderingDesc.m_renderArea = m_resolution;

    m_commandList->beginRendering(renderingDesc);
    setupTargetViewport();

    for (const DefaultRender::Item& item : m_renderItems)
    {
        renderItem(item);
    }

    m_commandList->endRendering();
    transitionRenderTarget(m_commandList, gpu::GraphicResourceState::CopySrc);
    m_commandList->end();

    submitCommandList(m_commandList);
    m_isPrepared = false;
}

bool ego::DefaultRender::isInitialized() const
{
    return m_isInitialized;
}

void ego::DefaultRender::wait()
{
    if (m_frameFence)
    {
        m_frameFence->waitValue(m_frameFenceValue);
        return;
    }

    if (m_commandQueue)
    {
        m_commandQueue->waitIdle();
    }
}

void ego::DefaultRender::present(GraphicPresenter& _presenter)
{
    if (!copyRenderTargetToPresenter(_presenter))
    {
        wait();
        return;
    }

    wait();
    _presenter.present();
}

void ego::DefaultRender::setResolution(const RenderResolution& _resolution)
{
    EGO_ASSERT(_resolution.m_x != 0 && _resolution.m_y != 0);
    m_pendingResolution = _resolution;
}

const ego::RenderResolution& ego::DefaultRender::getResolution() const
{
    return m_pendingResolution;
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

void ego::DefaultRender::collectRenderItems(Level& _level)
{
    m_renderItems.clear();

    _level.forEachComponent<MeshRenderComponent>(
        [this, &_level](ecs::Entity _entity, const MeshRenderComponent& _meshRenderComponent)
        {
            if (!_meshRenderComponent.m_mesh || !_meshRenderComponent.m_material)
            {
                return;
            }

            DefaultRender::Item item;
            item.m_mesh = _meshRenderComponent.m_mesh;
            item.m_material = _meshRenderComponent.m_material;
            item.m_objectIndex = static_cast<uint32_t>(m_renderItems.size());

            const TransformComponent* transformComponent = _level.tryGetComponent<TransformComponent>(_entity);
            if (transformComponent)
            {
                item.m_globalTransform = transformComponent->m_globalTransform;
            }

            m_renderItems.push_back(item);
        }
    );
}

bool ego::DefaultRender::prepareRenderTarget()
{
    if (m_pendingResolution.m_x == 0 || m_pendingResolution.m_y == 0)
    {
        return false;
    }

    const gpu::Texture2DDesc* currentDesc = m_renderTargetTexture ? &m_renderTargetTexture->getDesc() : nullptr;
    if (currentDesc &&
        currentDesc->m_size.m_x == m_pendingResolution.m_x &&
        currentDesc->m_size.m_y == m_pendingResolution.m_y &&
        currentDesc->m_format == RenderTargetFormat &&
        m_renderTargetView)
    {
        m_resolution = m_pendingResolution;
        return true;
    }

    m_resolution = m_pendingResolution;

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(
        gpu::TextureUsageRenderTarget | gpu::GraphicResourceUsageTransferSrc
    );
    textureDesc.m_size = m_resolution;
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_samples.m_quality = 0;
    textureDesc.m_format = RenderTargetFormat;

    m_renderTargetTexture = engine::GetEngine().getGraphicDevice().createTexture2D(textureDesc);
    EGO_CHECK_RETURN_FALSE(m_renderTargetTexture);

    gpu::TextureViewDesc viewDesc;
    viewDesc.m_type = gpu::GraphicResourceViewType::RenderTarget;
    viewDesc.m_dimension = gpu::TextureViewDimension::D2;
    viewDesc.m_format = textureDesc.m_format;

    m_renderTargetView = engine::GetEngine().getGraphicDevice().createTextureView(m_renderTargetTexture, viewDesc);
    m_renderTargetState = gpu::GraphicResourceState::Common;

    return static_cast<bool>(m_renderTargetView);
}

bool ego::DefaultRender::prepareShaderData(Level& _level, ecs::Entity _cameraEntity)
{
    return prepareCameraShaderData(_level, _cameraEntity) && prepareObjectShaderData();
}

bool ego::DefaultRender::prepareCameraShaderData(Level& _level, ecs::Entity _cameraEntity)
{
    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();

    if (!m_cameraShaderDataBuffer)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageConstantBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(
            gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead
        );
        bufferDesc.m_size = ego::Align(static_cast<uint32_t>(sizeof(CameraShaderData)), ConstantBufferAlignment);
        bufferDesc.m_stride = sizeof(CameraShaderData);

        const gpu::BufferReference buffer = graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(buffer);

        gpu::BufferViewDesc viewDesc;
        viewDesc.m_type = gpu::GraphicResourceViewType::ConstantBuffer;
        viewDesc.m_size = sizeof(CameraShaderData);

        const gpu::BufferViewReference view = graphicDevice.createBufferView(buffer, viewDesc);
        EGO_CHECK_RETURN_FALSE(view && view->getBindlessIndex() != gpu::InvalidBindlessIndex);

        m_cameraShaderDataBuffer = buffer;
        m_cameraShaderDataView = view;
    }

    const CameraComponent* cameraComponent = _level.tryGetComponent<CameraComponent>(_cameraEntity);
    const TransformComponent* transformComponent = _level.tryGetComponent<TransformComponent>(_cameraEntity);
    if (!cameraComponent || !transformComponent)
    {
        return false;
    }

    const Transform& cameraTransform = transformComponent->m_globalTransform;
    const ComputeMatrix4x4& projectionMatrix = cameraComponent->m_projection;
    const ComputeMatrix4x4 viewMatrix = InvertComputeMatrix4x4(cameraTransform.m_matrix);
    const ComputeMatrix4x4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    const FloatVector3 cameraPosition = cameraTransform.getOrigin().getFloatVector3();
    m_cameraViewProjectionMatrix = viewProjectionMatrix;

    CameraShaderData shaderData;
    shaderData.m_view = viewMatrix.getFloatMatrix4x4();
    shaderData.m_projection = projectionMatrix.getFloatMatrix4x4();
    shaderData.m_viewProjection = viewProjectionMatrix.getFloatMatrix4x4();
    shaderData.m_position = FloatVector4(cameraPosition, 1.0f);

    void* mappedData = m_cameraShaderDataBuffer->map(0, sizeof(shaderData));
    EGO_CHECK_RETURN_FALSE(mappedData);

    std::memcpy(mappedData, &shaderData, sizeof(shaderData));
    m_cameraShaderDataBuffer->unmap(0, sizeof(shaderData));
    return true;
}

bool ego::DefaultRender::prepareObjectShaderData()
{
    if (m_renderItems.empty())
    {
        return true;
    }

    GraphicDevice& graphicDevice = engine::GetEngine().getGraphicDevice();
    const uint32_t requiredCapacity = static_cast<uint32_t>(m_renderItems.size());

    if (!m_objectShaderDataBuffer || m_objectShaderDataCapacity < requiredCapacity)
    {
        const uint32_t newCapacity = (std::max)(requiredCapacity, (std::max)(m_objectShaderDataCapacity * 2, 1u));

        gpu::BufferDesc bufferDesc;
        bufferDesc.m_usage = gpu::GraphicResourceUsageShaderResource;
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(
            gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead
        );
        bufferDesc.m_size = static_cast<uint32_t>(sizeof(ObjectShaderData)) * newCapacity;
        bufferDesc.m_stride = sizeof(ObjectShaderData);

        const gpu::BufferReference buffer = graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(buffer);

        gpu::BufferViewDesc viewDesc;
        viewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
        viewDesc.m_size = bufferDesc.m_size;
        viewDesc.m_stride = sizeof(ObjectShaderData);

        const gpu::BufferViewReference view = graphicDevice.createBufferView(buffer, viewDesc);
        EGO_CHECK_RETURN_FALSE(view && view->getBindlessIndex() != gpu::InvalidBindlessIndex);

        m_objectShaderDataBuffer = buffer;
        m_objectShaderDataView = view;
        m_objectShaderDataCapacity = newCapacity;
    }

    const uint32_t dataSize = static_cast<uint32_t>(sizeof(ObjectShaderData)) * requiredCapacity;
    ObjectShaderData* shaderData = static_cast<ObjectShaderData*>(m_objectShaderDataBuffer->map(0, dataSize));
    EGO_CHECK_RETURN_FALSE(shaderData);

    const ComputeMatrix4x4 viewProjectionMatrix = m_cameraViewProjectionMatrix;

    for (uint32_t itemIndex = 0; itemIndex < requiredCapacity; ++itemIndex)
    {
        Item& item = m_renderItems[itemIndex];
        item.m_objectIndex = itemIndex;

        const ComputeMatrix4x4 modelMatrix = item.m_globalTransform.m_matrix;
        const ComputeMatrix4x4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

        shaderData[itemIndex].m_model = modelMatrix.getFloatMatrix4x4();
        shaderData[itemIndex].m_modelViewProjection = modelViewProjectionMatrix.getFloatMatrix4x4();
    }

    m_objectShaderDataBuffer->unmap(0, dataSize);
    return true;
}

void ego::DefaultRender::setupTargetViewport()
{
    EGO_ASSERT(m_renderTargetTexture);
    if (!m_renderTargetTexture)
    {
        return;
    }

    gpu::ViewportDesc viewportDesc;
    viewportDesc.m_width = static_cast<float>(m_resolution.m_x);
    viewportDesc.m_height = static_cast<float>(m_resolution.m_y);
    viewportDesc.m_minDepth = 0.0f;
    viewportDesc.m_maxDepth = 1.0f;
    m_commandList->setViewport(viewportDesc);

    gpu::ScissorRectDesc scissorRectDesc;
    scissorRectDesc.m_right = static_cast<int32_t>(m_resolution.m_x);
    scissorRectDesc.m_bottom = static_cast<int32_t>(m_resolution.m_y);
    m_commandList->setScissorRect(scissorRectDesc);
}

void ego::DefaultRender::submitCommandList(const gpu::GraphicCommandListReference& _commandList)
{
    if (!m_commandQueue || !_commandList)
    {
        return;
    }

    m_commandQueue->execute(_commandList);

    if (m_frameFence)
    {
        ++m_frameFenceValue;
        m_commandQueue->signal(m_frameFence, m_frameFenceValue);
    }
}

void ego::DefaultRender::transitionRenderTarget(
    const gpu::GraphicCommandListReference& _commandList,
    gpu::GraphicResourceState _nextState
)
{
    if (!_commandList || !m_renderTargetTexture || m_renderTargetState == _nextState)
    {
        return;
    }

    _commandList->resourceBarrier(m_renderTargetTexture, m_renderTargetState, _nextState);
    m_renderTargetState = _nextState;
}

bool ego::DefaultRender::copyRenderTargetToPresenter(GraphicPresenter& _presenter)
{
    EGO_ASSERT(m_commandQueue);
    EGO_ASSERT(m_presentCommandList);

    if (!m_commandQueue || !m_presentCommandList || !m_renderTargetTexture)
    {
        return false;
    }

    const gpu::Texture2DReference presenterTargetTexture = _presenter.getTargetTexture();
    if (!presenterTargetTexture)
    {
        return false;
    }

    const gpu::Texture2DDesc& renderTargetDesc = m_renderTargetTexture->getDesc();
    const gpu::Texture2DDesc& presenterTargetDesc = presenterTargetTexture->getDesc();
    if (renderTargetDesc.m_format != presenterTargetDesc.m_format)
    {
        return false;
    }

    const uint32_t copyWidth = (std::min)(renderTargetDesc.m_size.m_x, presenterTargetDesc.m_size.m_x);
    const uint32_t copyHeight = (std::min)(renderTargetDesc.m_size.m_y, presenterTargetDesc.m_size.m_y);
    if (copyWidth == 0 || copyHeight == 0)
    {
        return false;
    }

    gpu::TextureCopyRegionDesc copyRegion;
    copyRegion.m_extent = UInt32Vector3(copyWidth, copyHeight, 1);

    m_presentCommandList->begin();
    transitionRenderTarget(m_presentCommandList, gpu::GraphicResourceState::CopySrc);
    m_presentCommandList->resourceBarrier(
        presenterTargetTexture,
        gpu::GraphicResourceState::Present,
        gpu::GraphicResourceState::CopyDst
    );
    m_presentCommandList->copyTexture(m_renderTargetTexture, presenterTargetTexture, copyRegion);
    m_presentCommandList->resourceBarrier(
        presenterTargetTexture,
        gpu::GraphicResourceState::CopyDst,
        gpu::GraphicResourceState::Present
    );
    transitionRenderTarget(m_presentCommandList, gpu::GraphicResourceState::Common);
    m_presentCommandList->end();

    submitCommandList(m_presentCommandList);
    return true;
}

void ego::DefaultRender::renderItem(const DefaultRender::Item& _item)
{
    Mesh* mesh = _item.m_mesh.getObject();
    Material* material = _item.m_material.getObject();
    if (!mesh || !material)
    {
        return;
    }

    const gpu::GraphicPipelineReference& pipeline = material->getPipeline();
    if (!pipeline)
    {
        return;
    }

    m_commandList->setPipeline(pipeline);

    const Material::ResourceViewCollection& resourceViews = material->getResourceViews();
    for (uint32_t resourceViewIndex = 0; resourceViewIndex < resourceViews.size(); ++resourceViewIndex)
    {
        m_commandList->bindResourceView(resourceViewIndex, resourceViews[resourceViewIndex]);
    }

    const Material::SamplerCollection& samplers = material->getSamplers();
    for (uint32_t samplerIndex = 0; samplerIndex < samplers.size(); ++samplerIndex)
    {
        m_commandList->bindSampler(samplerIndex, samplers[samplerIndex]);
    }

    if (!m_cameraShaderDataView || !m_objectShaderDataView)
    {
        return;
    }

    RenderBindlessRootConstants rootConstants;
    rootConstants.m_cameraDataIndex = m_cameraShaderDataView->getBindlessIndex();
    rootConstants.m_objectDataIndex = m_objectShaderDataView->getBindlessIndex();
    rootConstants.m_objectIndex = _item.m_objectIndex;

    if (rootConstants.m_cameraDataIndex == gpu::InvalidBindlessIndex ||
        rootConstants.m_objectDataIndex == gpu::InvalidBindlessIndex)
    {
        return;
    }

    m_commandList->pushConstants(
        RenderBindlessRootConstantsStageFlag,
        RenderBindlessRootConstantsOffset,
        sizeof(rootConstants),
        &rootConstants
    );

    const Mesh::VertexBufferBinding& vertexBuffer = mesh->getVertexBuffer();
    if (vertexBuffer.m_buffer && vertexBuffer.m_stride != 0)
    {
        m_commandList->setVertexBuffer(0, vertexBuffer.m_buffer, vertexBuffer.m_stride, vertexBuffer.m_offset);
    }

    const Mesh::IndexBufferBinding& indexBuffer = mesh->getIndexBuffer();
    if (indexBuffer.m_buffer && mesh->getIndexCount() != 0)
    {
        m_commandList->setIndexBuffer(
            indexBuffer.m_buffer,
            indexBuffer.m_format,
            indexBuffer.m_offset
        );
        m_commandList->drawIndexed(mesh->getIndexCount());
        return;
    }

    if (mesh->getVertexCount() != 0)
    {
        m_commandList->draw(mesh->getVertexCount());
    }
}
