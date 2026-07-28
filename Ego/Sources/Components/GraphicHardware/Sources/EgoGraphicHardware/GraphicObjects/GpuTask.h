#pragma once

#include <vector>

#include "Fence.h"
#include "GraphicObject.h"

namespace ego::gpu
{
    class CommandQueue;
    enum class CommandType;

    struct GpuSyncPoint final
    {
        CommandType m_queueType;
        FencePointer m_fence = nullptr;
        Fence::FenceValue m_value = 0;
    };

    class GpuTask : public MTCountable
    {
    public:
        GpuTask() = default;

        bool isComplete() const;
        void wait() const;
        void waitOnQueue(CommandQueue& _queue) const;

        void addSyncPoint(const GpuSyncPoint& _syncPoint);
        void addKeepAliveObject(const GraphicObjectPointer& _object);

        const std::vector<GpuSyncPoint>& getSyncPoints() const;

    private:
        std::vector<GpuSyncPoint> m_syncPoints;
        std::vector<GraphicObjectPointer> m_keepAliveObjects;
    };

    EGO_INTRUSIVE_POINTER(GpuTask);

    enum class GpuCompletionMode
    {
        ReturnImmediately,
        WaitForCompletion
    };

    struct GpuOperationOptions final
    {
        GpuCompletionMode m_completionMode = GpuCompletionMode::ReturnImmediately;

        bool shouldWait() const
        {
            return m_completionMode == GpuCompletionMode::WaitForCompletion;
        }
    };

    template <typename TResourcePointer>
    struct GpuResourceTicket final
    {
        TResourcePointer m_resource = nullptr;
        GpuTaskPointer m_readyTask = nullptr;

        bool isReady() const
        {
            return m_resource && (!m_readyTask || m_readyTask->isComplete()) && m_resource->isGpuReady();
        }

        void waitReady() const
        {
            if (m_readyTask)
            {
                m_readyTask->wait();
            }
            else if (m_resource)
            {
                m_resource->waitGpuReady();
            }
        }

        explicit operator bool() const
        {
            return static_cast<bool>(m_resource);
        }
    };
} // namespace ego::gpu
