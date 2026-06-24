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
        FenceReference m_fence = nullptr;
        Fence::FenceValue m_value = 0;
    };

    class GpuTask : public STDDestroyMTCountable
    {
    public:
        GpuTask() = default;

        bool isComplete() const;
        void wait() const;
        void waitOnQueue(CommandQueue& _queue) const;

        void addSyncPoint(const GpuSyncPoint& _syncPoint);
        void addKeepAliveObject(const GraphicObjectReference& _object);

        const std::vector<GpuSyncPoint>& getSyncPoints() const;

    private:
        std::vector<GpuSyncPoint> m_syncPoints;
        std::vector<GraphicObjectReference> m_keepAliveObjects;
    };

    EGO_REFERENCE(GpuTask);

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

    template <typename TResourceReference>
    struct GpuResourceTicket final
    {
        TResourceReference m_resource = nullptr;
        GpuTaskReference m_readyTask = nullptr;

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
