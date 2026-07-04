#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoRuntime/Job/JobGraphDescriptor.h"

namespace ego::engine
{
    class MainLoop final : public NonCopyable
    {
    public:
        struct FrameJobs final
        {
            JobDescriptorID m_beginJobID;
            JobDescriptorID m_endJobID;
        };

        struct RenderJobs final
        {
            JobDescriptorID m_beginJobID;
            JobDescriptorID m_renderJobID;
            JobDescriptorID m_presentJobID;
            JobDescriptorID m_endJobID;
        };

        struct FrameLogicJobs final
        {
            JobDescriptorID m_beginJobID;
            JobDescriptorID m_endJobID;
        };

        struct PrepareRenderJobs final
        {
            JobDescriptorID m_beginJobID;
            JobDescriptorID m_prepareJobID;
            JobDescriptorID m_endJobID;
        };

        using JobDescriptorIDCollection = JobGraphDescriptorBuilder::JobDescriptorIDCollection;

        MainLoop() = default;
        ~MainLoop() override
        {
            release();
        }

        bool init(const JobDescriptor& _renderFrameJobDescriptor, const JobDescriptor& _presentFrameJobDescriptor, const JobDescriptor& _prepareRenderFrameJobDescriptor);
        void release();

        JobDescriptorID addJob(const JobDescriptor& _descriptor);
        bool removeJob(JobDescriptorID _jobID);

        JobDescriptorID addJobBefore(const JobDescriptor& _descriptor, JobDescriptorID _childJobID);
        JobDescriptorID addJobBefore(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _childJobIDs);
        JobDescriptorID addJobAfter(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID);
        JobDescriptorID addJobAfter(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _parentJobIDs);
        JobDescriptorID addJobBetween(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID);
        JobDescriptorID addJobBetween(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _parentJobIDs, const JobDescriptorIDCollection& _childJobIDs);

        JobDescriptorID addJobGraph(const JobGraphDescriptorPointer& _descriptor);
        JobDescriptorID addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _childJobID);
        JobDescriptorID addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _childJobIDs);
        JobDescriptorID addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID);
        JobDescriptorID addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _parentJobIDs);
        JobDescriptorID addJobGraphBetween(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID);
        JobDescriptorID addJobGraphBetween(
            const JobGraphDescriptorPointer& _descriptor,
            const JobDescriptorIDCollection& _parentJobIDs,
            const JobDescriptorIDCollection& _childJobIDs);

        void makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID);

        const FrameJobs& getFrameJobs() const;
        JobDescriptorID getFrameBeginJobID() const;
        JobDescriptorID getFrameEndJobID() const;

        const RenderJobs& getRenderJobs() const;
        JobDescriptorID getRenderBeginJobID() const;
        JobDescriptorID getRenderJobID() const;
        JobDescriptorID getPresentJobID() const;
        JobDescriptorID getRenderEndJobID() const;

        const FrameLogicJobs& getFrameLogicJobs() const;
        JobDescriptorID getFrameLogicBeginJobID() const;
        JobDescriptorID getFrameLogicEndJobID() const;

        const PrepareRenderJobs& getPrepareRenderJobs() const;
        JobDescriptorID getPrepareRenderBeginJobID() const;
        JobDescriptorID getPrepareRenderJobID() const;
        JobDescriptorID getPrepareRenderEndJobID() const;

        JobGraphReference createJobGraph() const;

        bool isInitialized() const;

    private:
        JobGraphDescriptorBuilder m_jobGraphDescriptorBuilder;

        FrameJobs m_frameJobs;
        RenderJobs m_renderJobs;
        FrameLogicJobs m_frameLogicJobs;
        PrepareRenderJobs m_prepareRenderJobs;

        bool m_isInitialized = false;
    };
} // namespace ego::engine
