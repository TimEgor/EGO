#include "DefaultRender.h"

#include <algorithm>
#include <cstring>

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Graphic/Presenter/GraphicPresenter.h"
#include "EgoEngine/Graphic/Render/Component/CameraComponent.h"
#include "EgoEngine/Graphic/Render/Component/MeshRenderComponent.h"
#include "EgoEngine/Graphic/Render/RenderShaderData.h"
#include "EgoEngine/Level/Level.h"

namespace
{
    constexpr uint32_t ConstantBufferAlignment = 256;

    uint32_t AlignTo(uint32_t _value, uint32_t _alignment)
    {
        return _alignment ? ((_value + _alignment - 1) / _alignment) * _alignment : _value;
    }
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

    m_commandList = nullptr;
    m_commandQueue = nullptr;

    m_isInitialized = false;
}

void ego::DefaultRender::clearResources()
{
    wait();

    m_renderItems.clear();
}

void ego::DefaultRender::render(GraphicPresenter& _presenter, Level& _level, ecs::Entity _cameraEntity)
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

    collectRenderItems(_level);

    if (!prepareShaderData(_level, _cameraEntity))
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
        bufferDesc.m_size = AlignTo(static_cast<uint32_t>(sizeof(CameraShaderData)), ConstantBufferAlignment);
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
