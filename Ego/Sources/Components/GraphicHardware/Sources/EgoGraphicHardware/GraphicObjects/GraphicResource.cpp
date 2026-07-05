#include "GraphicResource.h"

ego::gpu::InitialGraphicResourceData::InitialGraphicResourceData(const void* _data, uint32_t _dataSize, uint32_t _rowPitch, uint32_t _slicePitch)
    : m_data(_data),
      m_dataSize(_dataSize),
      m_rowPitch(_rowPitch),
      m_slicePitch(_slicePitch)
{
}

bool ego::gpu::InitialGraphicResourceData::isValid() const
{
    return m_data != nullptr && m_dataSize != 0;
}

bool ego::gpu::GraphicResource::isGpuReady() const
{
    return !m_lastWriteTask || m_lastWriteTask->isComplete();
}

void ego::gpu::GraphicResource::waitGpuReady() const
{
    if (m_lastWriteTask)
    {
        m_lastWriteTask->wait();
    }
}

const ego::gpu::GpuTaskReference& ego::gpu::GraphicResource::getLastWriteTask() const
{
    return m_lastWriteTask;
}

void ego::gpu::GraphicResource::setLastWriteTask(const GpuTaskReference& _task)
{
    m_lastWriteTask = _task;
}

ego::gpu::GraphicResourceState ego::gpu::GraphicResource::getState() const
{
    return m_state;
}

void ego::gpu::GraphicResource::setState(GraphicResourceState _state)
{
    m_state = _state;
}
