#include "JobGraphDescriptor.h"

#include "EgoCore/Assert/AssertCore.h"

#include <atomic>
#include <queue>

namespace
{
    std::atomic<uint32_t> g_lastJobGraphDescriptorBuilderOwnerID = 0;

    uint32_t CreateJobGraphDescriptorBuilderOwnerID()
    {
        const uint32_t ownerID = g_lastJobGraphDescriptorBuilderOwnerID.fetch_add(1) + 1;
        EGO_ASSERT(ownerID != 0);

        return ownerID;
    }
}

ego::JobDescriptorID::JobDescriptorID(uint32_t _ownerID, uint32_t _index)
    : m_ownerID(_ownerID),
      m_index(_index)
{}

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
    return m_ownerID == _id.m_ownerID && m_index == _id.m_index;
}

bool ego::JobDescriptorID::operator!=(JobDescriptorID _id) const
{
    return !(*this == _id);
}

ego::JobGraphDescriptor::JobGraphDescriptor(
    const NodeCollection& _nodes,
    const DependencyCollection& _dependencies
)
    : m_nodes(_nodes),
      m_dependencies(_dependencies)
{}

ego::JobGraphReference ego::JobGraphDescriptor::createJobGraph() const
{
    if (m_nodes.empty())
    {
        return nullptr;
    }

    JobGraphBuilder graphBuilder;
    std::vector<JobGraphBuilder::JobGraphJobID> graphJobIDs;
    graphJobIDs.reserve(m_nodes.size());

    for (const Node& node : m_nodes)
    {
        JobGraphBuilder::JobGraphJobID graphJobID(JobGraphBuilder::InvalidJobGraphJobIndex);

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

        if (graphJobID.m_index == JobGraphBuilder::InvalidJobGraphJobIndex)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor failed to add node to graph.");
            return nullptr;
        }

        graphJobIDs.push_back(graphJobID);
    }

    for (const Dependency& dependency : m_dependencies)
    {
        if (dependency.m_parentJobID.getIndex() >= graphJobIDs.size()
            || dependency.m_childJobID.getIndex() >= graphJobIDs.size())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor dependency is invalid.");
            return nullptr;
        }

        graphBuilder.makeDependency(
            graphJobIDs[dependency.m_parentJobID.getIndex()],
            graphJobIDs[dependency.m_childJobID.getIndex()]
        );
    }

    return graphBuilder.getGraph();
}

bool ego::JobGraphDescriptor::isEmpty() const
{
    return m_nodes.empty();
}

ego::JobGraphDescriptorBuilder::JobGraphDescriptorBuilder()
    : m_ownerID(CreateJobGraphDescriptorBuilderOwnerID())
{}

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

    const JobDescriptorID id(m_ownerID, static_cast<uint32_t>(m_nodes.size()));
    m_nodes.push_back(node);

    return id;
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

    const JobDescriptorID id(m_ownerID, static_cast<uint32_t>(m_nodes.size()));
    m_nodes.push_back(node);

    return id;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobBefore(
    const JobDescriptor& _descriptor,
    JobDescriptorID _childJobID
)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobAfter(
    const JobDescriptor& _descriptor,
    JobDescriptorID _parentJobID
)
{
    const JobDescriptorID jobID = addJob(_descriptor);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphBefore(
    const JobGraphDescriptorPointer& _descriptor,
    JobDescriptorID _childJobID
)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobDescriptorID ego::JobGraphDescriptorBuilder::addJobGraphAfter(
    const JobGraphDescriptorPointer& _descriptor,
    JobDescriptorID _parentJobID
)
{
    const JobDescriptorID jobID = addJobGraph(_descriptor);
    makeDependency(_parentJobID, jobID);

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
    return _id.isValid()
        && _id.m_ownerID == m_ownerID
        && _id.m_index < m_nodes.size();
}

bool ego::JobGraphDescriptorBuilder::isEmpty() const
{
    return m_nodes.empty();
}

void ego::JobGraphDescriptorBuilder::clear()
{
    m_ownerID = CreateJobGraphDescriptorBuilderOwnerID();
    m_nodes.clear();
    m_dependencies.clear();
}

ego::JobGraphDescriptorPointer ego::JobGraphDescriptorBuilder::build() const
{
    if (m_nodes.empty())
    {
        return nullptr;
    }

    if (!validate())
    {
        return nullptr;
    }

    return JobGraphDescriptorPointer(new JobGraphDescriptor(m_nodes, m_dependencies));
}

bool ego::JobGraphDescriptorBuilder::hasDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID) const
{
    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (dependency.m_parentJobID == _parentJobID && dependency.m_childJobID == _childJobID)
        {
            return true;
        }
    }

    return false;
}

bool ego::JobGraphDescriptorBuilder::hasCycles() const
{
    std::vector<uint32_t> parentCounters(m_nodes.size(), 0);
    std::vector<std::vector<uint32_t>> childJobs(m_nodes.size());

    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        childJobs[dependency.m_parentJobID.m_index].push_back(dependency.m_childJobID.m_index);
        ++parentCounters[dependency.m_childJobID.m_index];
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

        if (node.m_type == JobGraphDescriptor::NodeType::JobGraph
            && (!node.m_jobGraphDescriptor || node.m_jobGraphDescriptor->isEmpty()))
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains invalid nested graph descriptor.");
            return false;
        }
    }

    for (const JobGraphDescriptor::Dependency& dependency : m_dependencies)
    {
        if (!isValid(dependency.m_parentJobID) || !isValid(dependency.m_childJobID))
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph descriptor contains invalid dependency.");
            return false;
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
