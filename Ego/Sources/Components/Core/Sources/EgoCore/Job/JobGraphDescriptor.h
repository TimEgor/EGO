#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "JobDescriptor.h"
#include "JobGraph.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ego
{
    class JobGraphDescriptor;
    EGO_POINTER(JobGraphDescriptor);
    EGO_WEAK_POINTER(JobGraphDescriptor);

    struct JobDescriptorID final
    {
        static constexpr uint32_t InvalidIndex = -1;
        static constexpr uint64_t BaseGeneration = 0;

        JobDescriptorID() = default;

        bool isValid() const;
        uint32_t getIndex() const;

        bool operator==(JobDescriptorID _id) const;
        bool operator!=(JobDescriptorID _id) const;

    private:
        friend class JobGraphDescriptorBuilder;

        JobDescriptorID(uint32_t _ownerID, uint32_t _index, uint64_t _generation);

        uint32_t m_ownerID = 0;
        uint32_t m_index = InvalidIndex;
        uint64_t m_generation = BaseGeneration;
    };

    class JobGraphDescriptor final
    {
    public:
        JobGraphReference createJobGraph() const;

        bool isEmpty() const;
        const char* getDbgName() const { return m_dbgName.c_str(); }

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
            uint32_t m_slotIndex = JobDescriptorID::InvalidIndex;
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
            const DependencyCollection& _dependencies,
            const char* _dbgName
        );

        NodeCollection m_nodes;
        DependencyCollection m_dependencies;
        std::string m_dbgName;
    };

    class JobGraphDescriptorBuilder final : public NonCopyable
    {
    public:
        using JobDescriptorIDCollection = std::vector<JobDescriptorID>;

        explicit JobGraphDescriptorBuilder(const char* _dbgName = nullptr);

        void setDbgName(const char* _dbgName);
        const char* getDbgName() const { return m_dbgName.c_str(); }

        JobDescriptorID addJob(const JobDescriptor& _descriptor);
        bool removeJob(JobDescriptorID _jobID);

        JobDescriptorID addJobBefore(const JobDescriptor& _descriptor, JobDescriptorID _childJobID);
        JobDescriptorID addJobBefore(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _childJobIDs);
        JobDescriptorID addJobAfter(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID);
        JobDescriptorID addJobAfter(
            const JobDescriptor& _descriptor,
            const JobDescriptorIDCollection& _parentJobIDs
        );
        JobDescriptorID addJobBetween(
            const JobDescriptor& _descriptor,
            JobDescriptorID _parentJobID,
            JobDescriptorID _childJobID
        );
        JobDescriptorID addJobBetween(
            const JobDescriptor& _descriptor,
            const JobDescriptorIDCollection& _parentJobIDs,
            const JobDescriptorIDCollection& _childJobIDs
        );

        JobDescriptorID addJobGraph(const JobGraphDescriptorPointer& _descriptor);
        JobDescriptorID addJobGraphBefore(
            const JobGraphDescriptorPointer& _descriptor,
            JobDescriptorID _childJobID
        );
        JobDescriptorID addJobGraphBefore(
            const JobGraphDescriptorPointer& _descriptor,
            const JobDescriptorIDCollection& _childJobIDs
        );
        JobDescriptorID addJobGraphAfter(
            const JobGraphDescriptorPointer& _descriptor,
            JobDescriptorID _parentJobID
        );
        JobDescriptorID addJobGraphAfter(
            const JobGraphDescriptorPointer& _descriptor,
            const JobDescriptorIDCollection& _parentJobIDs
        );
        JobDescriptorID addJobGraphBetween(
            const JobGraphDescriptorPointer& _descriptor,
            JobDescriptorID _parentJobID,
            JobDescriptorID _childJobID
        );
        JobDescriptorID addJobGraphBetween(
            const JobGraphDescriptorPointer& _descriptor,
            const JobDescriptorIDCollection& _parentJobIDs,
            const JobDescriptorIDCollection& _childJobIDs
        );

        void makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID);

        bool isValid(JobDescriptorID _id) const;
        bool isEmpty() const;

        void clear();

        JobGraphDescriptorPointer build() const;

    private:
        static constexpr uint32_t InvalidNodeIndex = JobDescriptorID::InvalidIndex;

        struct NodeSlot final
        {
            uint32_t m_nodeIndex = InvalidNodeIndex;
            uint32_t m_nextFreeNodeIndex = InvalidNodeIndex;
            uint64_t m_generation = JobDescriptorID::BaseGeneration;
        };

        using NodeSlotCollection = std::vector<NodeSlot>;

        JobDescriptorID addNode(const JobGraphDescriptor::Node& _node);
        bool hasDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID) const;
        bool hasCycles() const;
        bool validate() const;
        bool isNodeSlotIndexOwned(JobDescriptorID _id) const;
        bool isOwnedNodeID(JobDescriptorID _id) const;
        bool isActiveNodeID(JobDescriptorID _id) const;
        bool isActiveDependency(const JobGraphDescriptor::Dependency& _dependency) const;
        uint32_t getActiveNodeCount() const;
        void makeDependenciesBefore(JobDescriptorID _parentJobID, const JobDescriptorIDCollection& _childJobIDs);
        void makeDependenciesAfter(const JobDescriptorIDCollection& _parentJobIDs, JobDescriptorID _childJobID);

        uint32_t m_ownerID = 0;
        uint32_t m_firstFreeNodeIndex = InvalidNodeIndex;
        NodeSlotCollection m_nodeSlots;
        std::vector<JobGraphDescriptor::Node> m_nodes;
        std::vector<JobGraphDescriptor::Dependency> m_dependencies;
        std::string m_dbgName;
    };
}
