#include "GraphicDevice.h"

ego::gpu::GpuBufferTicket ego::GraphicDevice::createAndUploadBuffer(
    const gpu::BufferDesc& _desc,
    const gpu::InitialGraphicResourceData& _initialData,
    const gpu::GpuOperationOptions& _options)
{
    gpu::BufferReference buffer = createBuffer(_desc);
    if (!buffer)
    {
        return gpu::GpuBufferTicket();
    }

    if (!_initialData.isValid())
    {
        return gpu::GpuBufferTicket{buffer, nullptr};
    }

    gpu::GpuTaskReference readyTask = uploadBuffer(buffer, _initialData, _options);
    if (!readyTask)
    {
        return gpu::GpuBufferTicket();
    }

    return gpu::GpuBufferTicket{buffer, readyTask};
}

ego::gpu::GpuTexture2DTicket ego::GraphicDevice::createAndUploadTexture2D(
    const gpu::Texture2DDesc& _desc,
    const gpu::InitialGraphicResourceData& _initialData,
    const gpu::GpuOperationOptions& _options)
{
    gpu::Texture2DReference texture = createTexture2D(_desc);
    if (!texture)
    {
        return gpu::GpuTexture2DTicket();
    }

    if (!_initialData.isValid())
    {
        return gpu::GpuTexture2DTicket{texture, nullptr};
    }

    gpu::GpuTaskReference readyTask = uploadTexture2D(texture, _initialData, _options);
    if (!readyTask)
    {
        return gpu::GpuTexture2DTicket();
    }

    return gpu::GpuTexture2DTicket{texture, readyTask};
}
