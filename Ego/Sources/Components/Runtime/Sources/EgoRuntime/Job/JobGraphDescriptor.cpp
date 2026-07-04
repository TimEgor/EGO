#include "JobGraphDescriptor.h"

#include <atomic>
#include <queue>

#include "EgoCore/Assert/AssertCore.h"

namespace
{
    constexpr auto DefaultJobGraphDescriptorDbgName = "JobGraph";

    std::atomic<uint32_t> g_lastJobGraphDescriptorBuilderOwnerID = 0;

    const char* NormalizeJobGraphDescriptorDbgName(const char* _dbgName)
    {
        return (_dbgName && _dbgName[0] != '\0') ? _dbgName : DefaultJobGraphDescriptorDbgName;
    }

    uint32_t CreateJobGraphDescriptorBuilderOwnerID()
    {
        const uint32_t ownerID = g_lastJobGraphDescriptorBuilderOwnerID.fetch_add(1) + 1;
        EGO_ASSERT(ownerID != 0);

        return ownerID;
    }
} // namespace

ego::JobDescriptorID::JobDescriptorID(uint32_t _ownerID, uint32_t _index, uint64_t _generation)
    : m_ownerID(_ownerID),
      m_index(_index),
      m_generation(_generation)
{
}

bool ego::JobDescriptorID::isValid() const
{
    return m_ownerID != 0 && m_index != InvalidIndex;
}

uint32_t ego::JobDescriptorID::getIndex() const
{
    return m_index;
}

bool ego::JobDescriptorID::operator==(JobDescriptorID _id) const
{
    return m_ownerID == _id.m_ownerID && m_index == _id.m_index && m_generation == _id.m_generation;
}

bool ego::JobDescriptorID::operator!=(JobDescriptorID _id) const
{
    return !(*this == _id);
}

ego::JobGraphDescriptor::JobGraphDescriptor(const NodeCollection& _nodes, const DependencyCollection& _dependencies, const char* _dbgName)
    : m_nodes(_nodes),
      m_dependencies(_dependencies),
      m_dbgName(NormalizeJobGraphDescriptorDbgName(_dbgName))
{
}

ego::JobGraphReference ego::JobGraphDescriptor::createJobGraph() const
{
    if (m_nodes.empty())
    {
        return nullptr;
    }

    JobGraphBuilder graphBuilder(getDbgName());
    std::vector<JobGraphBuilder::JobGraphJobID> graphJobIDs;
    graphJobIDs.reserve(m_nodes.size());

    for (const Node& node : m_nodes)
    {
        JobGraphBuilder::JobGraphJobID graphJobID;

        if (node.m_type == NodeType::Job)
        {
            const JobReference job = node.m_jobDescriptor.createJob();
            if (!job)
            {
                EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor failed to create job.");
                return nullptr;
            }

            graphJobID = graphBuilder.addJob(job);
        }
        else
        {
            if (!node.m_jobGraphDescriptor)
            {
                EGO_ASSERT_FAIL_MESSAGE("Nested job graph descriptor is null.");
                return nullptr;
            }

            const JobGraphReference jobGraph = node.m_jobGraphDescriptor->createJobGraph();
            if (!jobGraph)
            {
                EGO_ASSERT_FAIL_MESSAGE("Nested job graph descriptor failed to create graph.");
                return nullptr;
            }

            graphJobID = graphBuilder.addJobGraph(jobGraph);
        }

        if (!graphJobID.isValid())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor failed to add node to graph.");
            return nullptr;
        }

        graphJobIDs.push_back(graphJobID);
    }

    for (const Dependency& dependency : m_dependencies)
    {
        if (dependency.m_parentJobID.getIndex() >= graphJobIDs.size() || dependency.m_childJobID.getIndex() >= graphJobIDs.size())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor dependency is invalid.");
            return nullptr;
        }

        graphBuilder.makeDependency(graphJobIDs[dependency.m_parentJobID.getIndex()], graphJobIDs[dependency.m_childJobID.getIndex()]);
    }

    return graphBuilder.getGraph();
}

bool ego::JobGraphDescriptor::isEmpty() const
{
    return m_nodes.empty();
}

ego::JobGraphDescriptorBuilder::JobGraphDescriptorBuilder(const char* _dbgName)
    : m_ownerID(CreateJobGraphDescriptorBuilderOwnerID())
{
    setDbgName(_dbgName);
}

void ego::JobGraphDescriptorBuilder::setDbgName(const char* _dbgName)
{
    m_dbgName = NormalizeJobGraphDescriptorDbgName(_dbgName);
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJob(const JobDescriptor& _descriptor)
{
    if (!_descriptor)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor is invalid.");
        return JobDescriptorID();
    }

    JobGraphDescriptor::Node node;
    node.m_type = JobGraphDescriptor::NodeType::Job;
    node.m_jobDescriptor = _descriptor;

    return addNode(node);
}

bool ego::JobGraphDescriptorBuilder::removeJob(JobDescriptorID _jobID)
{
    if (!isValid(_jobID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Job descriptor id is invalid.");
        return false;
    }

    NodeSlot& nodeSlot = m_nodeSlots[_jobID.m_index];
    const uint32_t nodeIndex = nodeSlot.m_nodeIndex;
    EGO_ASSERT(nodeIndex < m_nodes.size());

    const uint32_t lastNodeIndex = static_cast<uint32_t>(m_nodes.size() - 1);
    if (nodeIndex != lastNodeIndex)
    {
        m_nodes[nodeIndex] = m_nodes[lastNodeIndex];

        NodeSlot& movedNodeSlot = m_nodeSlots[m_nodes[nodeIndex].m_slotIndex];
        movedNodeSlot.m_nodeIndex = nodeIndex;
    }

    m_nodes.pop_back();

    nodeSlot.m_nodeIndex = InvalidNodeIndex;
    nodeSlot.m_nextFreeNodeIndex = m_firstFreeNodeIndex;
    m_firstFreeNodeIndex = _jobID.m_index;

    return true;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraph(const JobGraphDescriptorPointer& _descriptor)
{
    if (!_descriptor || _descriptor->isEmpty())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor is invalid.");
        return JobDescriptorID();
    }

    JobGraphDescriptor::Node node;
    node.m_type = JobGraphDescriptor::NodeType::JobGraph;
    node.m_jobGraphDescriptor = _descriptor;

    return addNode(node);
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobBefore(const JobDescriptor& _descriptor, JobDescriptorID _childJobID)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobBefore(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _childJobIDs)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobAfter(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobAfter(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _parentJobIDs)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependenciesAfter(_parentJobIDs, jobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobBetween(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependency(_parentJobID, jobID);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobBetween(
    const JobDescriptor& _descriptor,
    const JobDescriptorIDCollection& _parentJobIDs,
    const JobDescriptorIDCollection& _childJobIDs)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependenciesAfter(_parentJobIDs, jobID);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _childJobID)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _childJobIDs)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _parentJobIDs)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependenciesAfter(_parentJobIDs, jobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphBetween(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependency(_parentJobID, jobID);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphBetween(
    const JobGraphDescriptorPointer& _descriptor,
    const JobDescriptorIDCollection& _parentJobIDs,
    const JobDescriptorIDCollection& _childJobIDs)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependenciesAfter(_parentJobIDs, jobID);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

void ego::JobGraphDescriptorBuilder::makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    if (!isValid(_parentJobID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Parent job descriptor id is invalid.");
        return;
    }

    if (!isValid(_childJobID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Child job descriptor id is invalid.");
        return;
    }

    if (_parentJobID == _childJobID)
    {
        EGO_ASSERT_FAIL_MESSAGE("Child and parent job descriptor ids are same.");
        return;
    }

    if (hasDependency(_parentJobID, _childJobID))
    {
        return;
    }

    m_dependencies.push_back(JobGraphDescriptor::Dependency{_parentJobID, _childJobID});
}

bool ego::JobGraphDescriptorBuilder::isValid(JobDescriptorID _id) const
{
    return isActiveNodeID(_id);
}

bool ego::JobGraphDescriptorBuilder::isEmpty() const
{
    return m_nodes.empty();
}

void ego::JobGraphDescriptorBuilder::clear()
{
    m_ownerID = CreateJobGraphDescriptorBuilderOwnerID();
    m_firstFreeNodeIndex = InvalidNodeIndex;
    m_nodeSlots.clear();
    m_nodes.clear();
    m_dependencies.clear();
}

ego::JobGraphDescriptorPointer ego::JobGraphDescriptorBuilder::build() const
{
    if (isEmpty())
    {
        return nullptr;
    }

    if (!validate())
    {
        return nullptr;
    }

    std::vector<uint32_t> remappedNodeIndices(m_nodeSlots.size(), InvalidNodeIndex);
    std::vector<JobGraphDescriptor::Node> nodes;
    nodes.reserve(getActiveNodeCount());

    for (uint32_t nodeIndex = 0; nodeIndex < m_nodes.size(); ++nodeIndex)
    {
        JobGraphDescriptor::Node node = m_nodes[nodeIndex];

        remappedNodeIndices[node.m_slotIndex] = static_cast<uint32_t>(nodes.size());
        node.m_slotIndex = static_cast<uint32_t>(nodes.size());
        nodes.push_back(node);
    }

    std::vector<JobGraphDescriptor::Dependency> dependencies;
    dependencies.reserve(m_dependencies.size());
    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (!isActiveDependency(dependency))
        {
            continue;
        }

        const uint32_t parentJobIndex = remappedNodeIndices[dependency.m_parentJobID.m_index];
        const uint32_t childJobIndex = remappedNodeIndices[dependency.m_childJobID.m_index];
        if (parentJobIndex == InvalidNodeIndex || childJobIndex == InvalidNodeIndex)
        {
            continue;
        }

        dependencies.push_back(
            JobGraphDescriptor::Dependency{JobDescriptorID(m_ownerID, parentJobIndex, JobDescriptorID::BaseGeneration),
                JobDescriptorID(m_ownerID, childJobIndex, JobDescriptorID::BaseGeneration)});
    }

    return JobGraphDescriptorPointer(new JobGraphDescriptor(nodes, dependencies, getDbgName()));
}

bool ego::JobGraphDescriptorBuilder::hasDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID) const
{
    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (!isActiveDependency(dependency))
        {
            continue;
        }

        if (dependency.m_parentJobID == _parentJobID && dependency.m_childJobID == _childJobID)
        {
            return true;
        }
    }

    return false;
}

void ego::JobGraphDescriptorBuilder::makeDependenciesBefore(JobDescriptorID _parentJobID, const JobDescriptorIDCollection& _childJobIDs)
{
    for (const JobDescriptorID childJobID : _childJobIDs)
    {
        makeDependency(_parentJobID, childJobID);
    }
}

void ego::JobGraphDescriptorBuilder::makeDependenciesAfter(const JobDescriptorIDCollection& _parentJobIDs, JobDescriptorID _childJobID)
{
    for (const JobDescriptorID parentJobID : _parentJobIDs)
    {
        makeDependency(parentJobID, _childJobID);
    }
}

bool ego::JobGraphDescriptorBuilder::hasCycles() const
{
    std::vector<uint32_t> parentCounters(m_nodes.size(), 0);
    std::vector<std::vector<uint32_t>> childJobs(m_nodes.size());

    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (!isActiveDependency(dependency))
        {
            continue;
        }

        const NodeSlot& parentNodeSlot = m_nodeSlots[dependency.m_parentJobID.m_index];
        const NodeSlot& childNodeSlot = m_nodeSlots[dependency.m_childJobID.m_index];

        childJobs[parentNodeSlot.m_nodeIndex].push_back(childNodeSlot.m_nodeIndex);
        ++parentCounters[childNodeSlot.m_nodeIndex];
    }

    std::queue<uint32_t> readyJobs;
    for (uint32_t jobIndex = 0; jobIndex < parentCounters.size(); ++jobIndex)
    {
        if (parentCounters[jobIndex] == 0)
        {
            readyJobs.push(jobIndex);
        }
    }

    uint32_t visitedJobCount = 0;
    while (!readyJobs.empty())
    {
        const uint32_t jobIndex = readyJobs.front();
        readyJobs.pop();
        ++visitedJobCount;

        for (const uint32_t childJobIndex : childJobs[jobIndex])
        {
            if (--parentCounters[childJobIndex] == 0)
            {
                readyJobs.push(childJobIndex);
            }
        }
    }

    return visitedJobCount != m_nodes.size();
}

bool ego::JobGraphDescriptorBuilder::validate() const
{
    for (const JobGraphDescriptor::Node& node : m_nodes)
    {
        if (node.m_type == JobGraphDescriptor::NodeType::Job && !node.m_jobDescriptor)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains invalid job descriptor.");
            return false;
        }

        if (node.m_type == JobGraphDescriptor::NodeType::JobGraph && (!node.m_jobGraphDescriptor || node.m_jobGraphDescriptor->isEmpty()))
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains invalid nested graph descriptor.");
            return false;
        }
    }

    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (!isNodeSlotIndexOwned(dependency.m_parentJobID) || !isNodeSlotIndexOwned(dependency.m_childJobID))
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains invalid dependency.");
            return false;
        }

        if (!isActiveDependency(dependency))
        {
            continue;
        }

        if (dependency.m_parentJobID == dependency.m_childJobID)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains self dependency.");
            return false;
        }
    }

    if (hasCycles())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains dependency cycle.");
        return false;
    }

    return true;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addNode(const JobGraphDescriptor::Node& _node)
{
    uint32_t nodeSlotIndex = InvalidNodeIndex;
    if (m_firstFreeNodeIndex != InvalidNodeIndex)
    {
        nodeSlotIndex = m_firstFreeNodeIndex;

        NodeSlot& nodeSlot = m_nodeSlots[nodeSlotIndex];
        const uint32_t nextFreeNodeIndex = nodeSlot.m_nextFreeNodeIndex;

        ++nodeSlot.m_generation;
        nodeSlot.m_nextFreeNodeIndex = InvalidNodeIndex;
        m_firstFreeNodeIndex = nextFreeNodeIndex;
    }
    else
    {
        if (m_nodeSlots.size() >= InvalidNodeIndex)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor node count overflow.");
            return JobDescriptorID();
        }

        nodeSlotIndex = static_cast<uint32_t>(m_nodeSlots.size());

        NodeSlot nodeSlot;
        nodeSlot.m_generation = JobDescriptorID::BaseGeneration;
        nodeSlot.m_nextFreeNodeIndex = InvalidNodeIndex;
        m_nodeSlots.push_back(nodeSlot);
    }

    JobGraphDescriptor::Node node = _node;
    node.m_slotIndex = nodeSlotIndex;
    m_nodes.push_back(node);

    NodeSlot& nodeSlot = m_nodeSlots[nodeSlotIndex];
    nodeSlot.m_nodeIndex = static_cast<uint32_t>(m_nodes.size() - 1);

    return JobDescriptorID(m_ownerID, nodeSlotIndex, nodeSlot.m_generation);
}

bool ego::JobGraphDescriptorBuilder::isNodeSlotIndexOwned(JobDescriptorID _id) const
{
    return _id.isValid() && _id.m_ownerID == m_ownerID && _id.m_index < m_nodeSlots.size();
}

bool ego::JobGraphDescriptorBuilder::isOwnedNodeID(JobDescriptorID _id) const
{
    return isNodeSlotIndexOwned(_id) && m_nodeSlots[_id.m_index].m_generation == _id.m_generation;
}

bool ego::JobGraphDescriptorBuilder::isActiveNodeID(JobDescriptorID _id) const
{
    return isOwnedNodeID(_id) && m_nodeSlots[_id.m_index].m_nodeIndex != InvalidNodeIndex;
}

bool ego::JobGraphDescriptorBuilder::isActiveDependency(const JobGraphDescriptor::Dependency& _dependency) const
{
    return isActiveNodeID(_dependency.m_parentJobID) && isActiveNodeID(_dependency.m_childJobID);
}

uint32_t ego::JobGraphDescriptorBuilder::getActiveNodeCount() const
{
    return static_cast<uint32_t>(m_nodes.size());
}
