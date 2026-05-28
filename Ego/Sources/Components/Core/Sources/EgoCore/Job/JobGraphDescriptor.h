#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "JobDescriptor.h"
#include "JobGraph.h"

#include <cstdint>
#include <vector>

namespace ego
{
    class JobGraphDescriptor;
    EGO_POINTER(JobGraphDescriptor);
    EGO_WEAK_POINTER(JobGraphDescriptor);

    struct JobDescriptorID final
    {
        static constexpr uint32_t InvalidIndex = 0xFFFFFFFFu;

        JobDescriptorID() = default;

        bool isValid() const;
        uint32_t getIndex() const;

        bool operator==(JobDescriptorID _id) const;
        bool operator!=(JobDescriptorID _id) const;

    private:
        friend class JobGraphDescriptorBuilder;

        JobDescriptorID(uint32_t _ownerID, uint32_t _index);

        uint32_t m_ownerID = 0;
        uint32_t m_index = InvalidIndex;
    };

    class JobGraphDescriptor final
    {
    public:
        JobGraphReference createJobGraph() const;

        bool isEmpty() const;

    private:
        friend class JobGraphDescriptorBuilder;

        enum class NodeType
        {
            Job,
            JobGraph
        };

        struct Node final
        {
            NodeType m_type = NodeType::Job;
            JobDescriptor m_jobDescriptor;
            JobGraphDescriptorPointer m_jobGraphDescriptor;
        };

        struct Dependency final
        {
            JobDescriptorID m_parentJobID;
            JobDescriptorID m_childJobID;
        };

        using NodeCollection = std::vector<Node>;
        using DependencyCollection = std::vector<Dependency>;

        JobGraphDescriptor(
            const NodeCollection& _nodes,
            const DependencyCollection& _dependencies
        );

        NodeCollection m_nodes;
        DependencyCollection m_dependencies;
    };

    class JobGraphDescriptorBuilder final : public NonCopyable
    {
    public:
        JobGraphDescriptorBuilder();

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

        bool isValid(JobDescriptorID _id) const;
        bool isEmpty() const;

        void clear();

        JobGraphDescriptorPointer build() const;

    private:
        bool hasDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID) const;
        bool hasCycles() const;
        bool validate() const;

        uint32_t m_ownerID = 0;
        std::vector<JobGraphDescriptor::Node> m_nodes;
        std::vector<JobGraphDescriptor::Dependency> m_dependencies;
    };
}
