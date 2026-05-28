#pragma once

#include "EgoCore/Job/JobGraphDescriptor.h"
#include "EgoCore/Patterns/NonCopyable.h"

namespace ego::engine
{
    class MainLoop final : public NonCopyable
    {
    public:
        MainLoop() = default;
        ~MainLoop() { release(); }

        bool init(const JobDescriptor& _renderJobDescriptor);
        void release();

        JobDescriptorID addJob(const JobDescriptor& _descriptor);
        JobDescriptorID addJobBefore(const JobDescriptor& _descriptor, JobDescriptorID _childJobID);
        JobDescriptorID addJobAfter(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID);

        JobDescriptorID addJobGraph(const JobGraphDescriptorPointer& _descriptor);
        JobDescriptorID addJobGraphBefore(
            const JobGraphDescriptorPointer& _descriptor,
            JobDescriptorID _childJobID
        );
        JobDescriptorID addJobGraphAfter(
            const JobGraphDescriptorPointer& _descriptor,
            JobDescriptorID _parentJobID
        );

        void makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID);

        JobDescriptorID getRenderJobID() const;
        JobGraphReference createJobGraph() const;

        bool isInitialized() const;

    private:
        JobGraphDescriptorBuilder m_jobGraphDescriptorBuilder;
        JobDescriptorID m_renderJobID;
        bool m_isInitialized = false;
    };
}
