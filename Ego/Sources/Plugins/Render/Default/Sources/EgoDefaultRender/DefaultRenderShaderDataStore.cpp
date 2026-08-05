#include "DefaultRenderShaderDataStore.h"

#include <algorithm>
#include <cstring>

#include "EgoCore/Memory/Utils.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoEngine/Graphic/SceneRender/RenderCamera.h"

#include "DefaultRenderConstants.h"
#include "RenderShaderData.h"

bool ego::render::DefaultRenderShaderDataStore::prepare(
    GraphicDevice& _graphicDevice,
    const RenderCamera& _camera,
    DefaultRenderScene& _scene,
    const gpu::Texture2DSize& _resolution)
{
    return prepareCameraShaderData(_graphicDevice, _camera, _resolution) && prepareObjectShaderData(_graphicDevice, _scene);
}

void ego::render::DefaultRenderShaderDataStore::release()
{
    m_objectShaderDataView = nullptr;
    m_objectShaderDataBuffer = nullptr;
    m_objectShaderDataCapacity = 0;
    m_cameraShaderDataView = nullptr;
    m_cameraShaderDataBuffer = nullptr;
}

const ego::render::RenderBufferView& ego::render::DefaultRenderShaderDataStore::getCameraShaderDataView() const
{
    return m_cameraShaderDataView;
}

const ego::render::RenderBufferView& ego::render::DefaultRenderShaderDataStore::getObjectShaderDataView() const
{
    return m_objectShaderDataView;
}

bool ego::render::DefaultRenderShaderDataStore::prepareCameraShaderData(
    GraphicDevice& _graphicDevice,
    const RenderCamera& _camera,
    const gpu::Texture2DSize& _resolution)
{
    if (!m_cameraShaderDataBuffer)
    {
        gpu::BufferDesc bufferDesc;
        bufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageConstantBuffer);
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);
        bufferDesc.m_size = Align(static_cast<uint32_t>(sizeof(CameraShaderData)), DefaultRenderConstantBufferAlignment);
        bufferDesc.m_stride = sizeof(CameraShaderData);

        const RenderBuffer buffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(buffer);

        gpu::BufferViewDesc viewDesc;
        viewDesc.m_type = gpu::GraphicResourceViewType::ConstantBuffer;
        viewDesc.m_size = sizeof(CameraShaderData);

        const RenderBufferView view = _graphicDevice.createBufferView(buffer.getObject(), viewDesc);
        EGO_CHECK_RETURN_FALSE(view && view->getBindlessIndex() != gpu::InvalidBindlessIndex);

        m_cameraShaderDataBuffer = buffer;
        m_cameraShaderDataView = view;
    }

    const FloatMatrix4x4& projectionMatrix = _camera.getProjectionMatrix();
    const FloatMatrix4x4& viewMatrix = _camera.getViewMatrix();
    const FloatMatrix4x4& viewProjectionMatrix = _camera.getViewProjectionMatrix();
    const FloatVector3 cameraPosition = _camera.getTransform().getOrigin();

    CameraShaderData shaderData;
    shaderData.m_view = viewMatrix;
    shaderData.m_projection = projectionMatrix;
    shaderData.m_viewProjection = viewProjectionMatrix;
    shaderData.m_position = FloatVector4(cameraPosition, 1.0f);
    shaderData.m_screenSize = FloatVector4(static_cast<float>(_resolution.m_x), static_cast<float>(_resolution.m_y), 0.0f, 0.0f);
    shaderData.m_inverseViewProjection = _camera.getInverseViewProjectionMatrix();

    void* mappedData = m_cameraShaderDataBuffer->map(0, sizeof(shaderData));
    EGO_CHECK_RETURN_FALSE(mappedData);

    std::memcpy(mappedData, &shaderData, sizeof(shaderData));
    m_cameraShaderDataBuffer->unmap(0, sizeof(shaderData));
    return true;
}

bool ego::render::DefaultRenderShaderDataStore::prepareObjectShaderData(GraphicDevice& _graphicDevice, DefaultRenderScene& _scene)
{
    DefaultRenderScene::ItemCollection& renderItems = _scene.getItems();
    if (renderItems.empty())
    {
        return true;
    }

    const uint32_t requiredCapacity = static_cast<uint32_t>(renderItems.size());
    if (!m_objectShaderDataBuffer || m_objectShaderDataCapacity < requiredCapacity)
    {
        const uint32_t newCapacity = (std::max)(requiredCapacity, (std::max)(m_objectShaderDataCapacity * 2, 1u));

        gpu::BufferDesc bufferDesc;
        bufferDesc.m_usage = gpu::GraphicResourceUsageShaderResource;
        bufferDesc.m_access = static_cast<gpu::CommonGraphicResourceAccess>(gpu::GraphicResourceAccessCpuWrite | gpu::GraphicResourceAccessGpuRead);
        bufferDesc.m_size = static_cast<uint32_t>(sizeof(ObjectShaderData)) * newCapacity;
        bufferDesc.m_stride = sizeof(ObjectShaderData);

        const RenderBuffer buffer = _graphicDevice.createBuffer(bufferDesc);
        EGO_CHECK_RETURN_FALSE(buffer);

        gpu::BufferViewDesc viewDesc;
        viewDesc.m_type = gpu::GraphicResourceViewType::ShaderResource;
        viewDesc.m_size = bufferDesc.m_size;
        viewDesc.m_stride = sizeof(ObjectShaderData);

        const RenderBufferView view = _graphicDevice.createBufferView(buffer.getObject(), viewDesc);
        EGO_CHECK_RETURN_FALSE(view && view->getBindlessIndex() != gpu::InvalidBindlessIndex);

        m_objectShaderDataBuffer = buffer;
        m_objectShaderDataView = view;
        m_objectShaderDataCapacity = newCapacity;
    }

    const uint32_t dataSize = static_cast<uint32_t>(sizeof(ObjectShaderData)) * requiredCapacity;
    auto shaderData = static_cast<ObjectShaderData*>(m_objectShaderDataBuffer->map(0, dataSize));
    EGO_CHECK_RETURN_FALSE(shaderData);

    for (uint32_t itemIndex = 0; itemIndex < requiredCapacity; ++itemIndex)
    {
        DefaultRenderItem& item = renderItems[itemIndex];
        item.m_objectIndex = itemIndex;

        shaderData[itemIndex].m_model = item.m_globalTransform.m_matrix;
    }

    m_objectShaderDataBuffer->unmap(0, dataSize);
    return true;
}
