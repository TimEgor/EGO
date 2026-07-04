#include "GpuTask.h"

#include "CommandQueue.h"

bool ego::gpu::GpuTask::isComplete() const
{
    for (const GpuSyncPoint& syncPoint : m_syncPoints)
    {
        if (syncPoint.m_fence && syncPoint.m_fence->getCompletedValue() < syncPoint.m_value)
        {
            return false;
        }
    }

    return true;
}

void ego::gpu::GpuTask::wait() const
{
    for (const GpuSyncPoint& syncPoint : m_syncPoints)
    {
        if (syncPoint.m_fence)
        {
            syncPoint.m_fence->waitValue(syncPoint.m_value);
        }
    }
}

void ego::gpu::GpuTask::waitOnQueue(CommandQueue& _queue) const
{
    for (const GpuSyncPoint& syncPoint : m_syncPoints)
    {
        if (syncPoint.m_fence && syncPoint.m_fence->getCompletedValue() < syncPoint.m_value)
        {
            _queue.wait(syncPoint.m_fence, syncPoint.m_value);
        }
    }
}

void ego::gpu::GpuTask::addSyncPoint(const GpuSyncPoint& _syncPoint)
{
    if (_syncPoint.m_fence && _syncPoint.m_value != 0)
    {
        m_syncPoints.push_back(_syncPoint);
    }
}

void ego::gpu::GpuTask::addKeepAliveObject(const GraphicObjectReference& _object)
{
    if (_object)
    {
        m_keepAliveObjects.push_back(_object);
    }
}

const std::vector<ego::gpu::GpuSyncPoint>& ego::gpu::GpuTask::getSyncPoints() const
{
    return m_syncPoints;
}
