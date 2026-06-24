#include "JobGraph.h"

#include "EgoCore/Assert/AssertCore.h"

#include "JobController.h"

#include <queue>

namespace
{
    constexpr auto DefaultJobGraphDbgName = "JobGraph";

    const char* NormalizeJobGraphDbgName(const char* _dbgName)
    {
        return (_dbgName && _dbgName[0] != '\0') ? _dbgName : DefaultJobGraphDbgName;
    }

#ifdef EGO_JOB_DEBUG
    std::string MakeJobGraphBarrierDbgName(const char* _graphDbgName, bool _isBegin)
    {
        std::string dbgName = _isBegin ? "JobGraph begin: " : "JobGraph end: ";
        dbgName += NormalizeJobGraphDbgName(_graphDbgName);
        return dbgName;
    }

    const char* GetJobGraphBarrierDbgName(const ego::JobGraph::GraphJobDataReference& _graphData, ego::JobGraph::BarrierDependencyJob::Type _type)
    {
        if (!_graphData)
        {
            return _type == ego::JobGraph::BarrierDependencyJob::Type::Begin ? "JobGraph begin" : "JobGraph end";
        }

        return _type == ego::JobGraph::BarrierDependencyJob::Type::Begin ? _graphData->getBeginBarrierDbgName() : _graphData->getEndBarrierDbgName();
    }
#endif
} // namespace

ego::JobGraph::GraphJobData::GraphJobData(const char* _dbgName)
{
#ifdef EGO_JOB_DEBUG
    m_dbgName = NormalizeJobGraphDbgName(_dbgName);
    m_beginBarrierDbgName = MakeJobGraphBarrierDbgName(m_dbgName.c_str(), true);
    m_endBarrierDbgName = MakeJobGraphBarrierDbgName(m_dbgName.c_str(), false);
#endif
}

void ego::JobGraph::GraphJobData::completeJob()
{
    const uint32_t previousJobCounter = m_jobCounter.fetch_sub(1);
    if (previousJobCounter == 0)
    {
        m_jobCounter = 0;
        EGO_ASSERT_FAIL_MESSAGE("Job graph completion counter is invalid.");
        return;
    }

    if (previousJobCounter != 1)
    {
        return;
    }

    CompletionCallbackCollection completionCallbacks;
    {
        std::lock_guard locker(m_mutex);
        m_state = JobGraphState::Finished;
        completionCallbacks.swap(m_completionCallback);
    }

    for (const auto& callback : completionCallbacks)
    {
        callback();
    }

    m_completionNotifier.notify_all();
}

void ego::JobGraph::GraphJobData::addCompletionCallback(const CompletionCallback& _callback)
{
    if (!_callback)
    {
        EGO_ASSERT_FAIL_MESSAGE("Graph job completion callback is null.");
        return;
    }

    bool executeImmediately = false;
    {
        std::lock_guard locker(m_mutex);
        if (m_state == JobGraphState::Finished)
        {
            executeImmediately = true;
        }
        else
        {
            m_completionCallback.push_back(_callback);
        }
    }

    if (executeImmediately)
    {
        _callback();
    }
}

void ego::JobGraph::GraphJobData::setExecutionContext(const JobControllerWeakPointer& _controller)
{
    std::lock_guard locker(m_mutex);
    m_jobController = _controller;
}

void ego::JobGraph::GraphJobData::wait()
{
    std::unique_lock locker(m_mutex);

    if (m_state == JobGraphState::Finished)
    {
        return;
    }

    if (m_state == JobGraphState::Undefined)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph hasn't been scheduled.");
        return;
    }

    m_completionNotifier.wait(
        locker,
        [this]()
        {
            return m_state == JobGraphState::Finished;
        });
}

bool ego::JobGraph::GraphJobData::isFinished() const
{
    return m_state == JobGraphState::Finished;
}

void ego::JobGraph::DependencyJob::removeChildDependencies()
{
    JobControllerPointer jobController = m_graphData->getJobController();
    EGO_ASSERT(jobController);

    for (size_t jobIndex = 0; jobIndex < m_dependencyJobs.size();)
    {
        const DependencyJobReference& childJob = m_dependencyJobs[jobIndex];

        if (childJob->removeParentDependency())
        {
            jobController->addJob(childJob);

            if (m_dependencyJobs.size() > 1)
            {
                m_dependencyJobs[jobIndex] = m_dependencyJobs.back();
            }

            m_dependencyJobs.pop_back();
        }
        else
        {
            ++jobIndex;
        }
    }
}

bool ego::JobGraph::DependencyJob::removeParentDependency()
{
    return m_parentCounter.fetch_sub(1) == 1;
}

ego::JobGraph::DependencyJob::DependencyJob(const GraphJobDataReference& _graphData, const char* _dbgName)
    : Job(_dbgName),
      m_graphData(_graphData)
{
}

void ego::JobGraph::DependencyJob::operate()
{
    if (!executeDependencyJob())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph node hasn't been finished.");
    }

    removeChildDependencies();

    m_graphData->completeJob();
}

ego::JobGraph::JobDependencyJob::JobDependencyJob(const JobReference& _job, const GraphJobDataReference& _graphData)
    : DependencyJob(_graphData, "GraphJob"),
      m_job(_job)
{
}

bool ego::JobGraph::JobDependencyJob::executeDependencyJob()
{
    if (!m_job)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph node job is invalid.");
        return false;
    }

    const JobControllerPointer jobController = getJobController();
    EGO_ASSERT(jobController);

    if (m_job->getState() == JobState::Undefined)
    {
        m_job->setExecutionContext(jobController);
    }

    m_job->execute();
    return m_job->isFinished();
}

ego::JobGraph::BarrierDependencyJob::BarrierDependencyJob(const GraphJobDataReference& _graphData, Type _type)
    : DependencyJob(
          _graphData,
#ifdef EGO_JOB_DEBUG
          GetJobGraphBarrierDbgName(_graphData, _type)
#else
          "GraphBarrier"
#endif
              ),
      m_type(_type)
{
}

bool ego::JobGraph::BarrierDependencyJob::executeDependencyJob()
{
    return true;
}

ego::JobGraph::JobGraph(const DependencyJobReference& _entryJob, const DependencyJobReference& _exitJob, JobGraphCollection&& _nestedGraphs, GraphJobDataReference _data)
    : m_entryJob(_entryJob),
      m_exitJob(_exitJob),
      m_nestedGraphs(std::move(_nestedGraphs)),
      m_graphData(_data)
{
}

ego::JobGraph::GraphJobDataReference ego::JobGraph::getGraphData() const
{
    return m_graphData;
}

void ego::JobGraph::setExecutionContext(const JobControllerWeakPointer& _jobController)
{
    trySetExecutionContext(_jobController);
}

bool ego::JobGraph::trySetExecutionContext(const JobControllerWeakPointer& _jobController)
{
    if (!m_graphData)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph data is invalid.");
        return false;
    }

    if (!m_entryJob || !m_exitJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph boundary jobs are invalid.");
        return false;
    }

    for (const JobGraphReference& nestedGraph : m_nestedGraphs)
    {
        if (!nestedGraph)
        {
            EGO_ASSERT_FAIL_MESSAGE("Nested job graph is invalid.");
            return false;
        }

        if (nestedGraph->isScheduled())
        {
            EGO_ASSERT_FAIL_MESSAGE("Nested job graph has been already scheduled.");
            return false;
        }
    }

    auto expectedState = JobGraphState::Undefined;
    if (!m_graphData->m_state.compare_exchange_strong(expectedState, JobGraphState::Pending))
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph has been already scheduled.");
        return false;
    }

    m_graphData->setExecutionContext(_jobController);

    for (const JobGraphReference& nestedGraph : m_nestedGraphs)
    {
        if (!nestedGraph->trySetExecutionContext(_jobController))
        {
            return false;
        }
    }

    return true;
}

void ego::JobGraph::wait()
{
    if (!m_graphData)
    {
        return;
    }

    m_graphData->wait();
}

bool ego::JobGraph::isFinished() const
{
    return m_graphData && m_graphData->isFinished();
}

#ifdef EGO_JOB_DEBUG
const char* ego::JobGraph::getDbgName() const
{
    return m_graphData ? m_graphData->getDbgName() : nullptr;
}
#endif

bool ego::JobGraph::schedule(const JobControllerWeakPointer& _jobController, JobCollection& _baseJobs)
{
    if (!m_graphData)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph data is invalid.");
        return false;
    }

    if (!m_entryJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph entry job is invalid.");
        return false;
    }

    if (m_entryJob->m_parentCounter.load() != 0)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph has external parent dependencies.");
        return false;
    }

    if (!trySetExecutionContext(_jobController))
    {
        return false;
    }

    _baseJobs.clear();
    _baseJobs.push_back(m_entryJob);

    return true;
}

bool ego::JobGraph::isScheduled() const
{
    return m_graphData && m_graphData->getState() != JobGraphState::Undefined;
}

ego::JobGraphBuilder::JobGraphJobID::JobGraphJobID(const void* _owner, uint32_t _generation, uint32_t _index)
    : m_owner(_owner),
      m_generation(_generation),
      m_index(_index)
{
}

bool ego::JobGraphBuilder::JobGraphJobID::isValid() const
{
    return m_owner != nullptr && m_index != InvalidIndex;
}

uint32_t ego::JobGraphBuilder::JobGraphJobID::getIndex() const
{
    return m_index;
}

bool ego::JobGraphBuilder::JobGraphJobID::operator==(JobGraphJobID _id) const
{
    return m_owner == _id.m_owner && m_generation == _id.m_generation && m_index == _id.m_index;
}

bool ego::JobGraphBuilder::JobGraphJobID::operator!=(JobGraphJobID _id) const
{
    return !(*this == _id);
}

ego::JobGraphBuilder::GraphBuildingContext::GraphBuildingContext() = default;

ego::JobGraphBuilder::JobGraphBuilder(const char* _dbgName)
{
    setDbgName(_dbgName);
}

void ego::JobGraphBuilder::setDbgName(const char* _dbgName)
{
    m_dbgName = NormalizeJobGraphDbgName(_dbgName);
}

void ego::JobGraphBuilder::GraphBuildingContext::clear()
{
    m_nodes.clear();
    m_dependencies.clear();
    ++m_generation;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::GraphBuildingContext::addJob(const JobReference& _job)
{
    Node node;
    node.m_type = NodeType::Job;
    node.m_job = _job;

    const JobGraphJobID jobID(this, m_generation, static_cast<uint32_t>(m_nodes.size()));
    m_nodes.push_back(node);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::GraphBuildingContext::addJobGraph(const JobGraphReference& _jobGraph)
{
    Node node;
    node.m_type = NodeType::JobGraph;
    node.m_jobGraph = _jobGraph;

    const JobGraphJobID jobID(this, m_generation, static_cast<uint32_t>(m_nodes.size()));
    m_nodes.push_back(node);

    return jobID;
}

bool ego::JobGraphBuilder::GraphBuildingContext::isValid(JobGraphJobID _jobID) const
{
    return _jobID.isValid() && _jobID.m_owner == this && _jobID.m_generation == m_generation && _jobID.m_index < m_nodes.size();
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJob(const JobReference& _job)
{
    if (!_job)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job is invalid.");
        return JobGraphJobID();
    }

    if (_job->getState() != JobState::Undefined)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job has been already scheduled.");
        return JobGraphJobID();
    }

    return m_buildingContext.addJob(_job);
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobBefore(const JobReference& _job, JobGraphJobID _childJobID)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobBefore(const JobReference& _job, const JobGraphJobIDCollection& _childJobIDs)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobAfter(const JobReference& _job, JobGraphJobID _parentJobID)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobAfter(const JobReference& _job, const JobGraphJobIDCollection& _parentJobIDs)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependenciesAfter(_parentJobIDs, jobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobBetween(const JobReference& _job, JobGraphJobID _parentJobID, JobGraphJobID _childJobID)
{
    const JobGraphJobID jobID = addJob(_job);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependency(_parentJobID, jobID);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobBetween(
    const JobReference& _job,
    const JobGraphJobIDCollection& _parentJobIDs,
    const JobGraphJobIDCollection& _childJobIDs)
{
    const JobGraphJobID jobID = addJob(_job);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependenciesAfter(_parentJobIDs, jobID);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraph(const JobGraphReference& _graph)
{
    if (!_graph)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph is invalid.");
        return JobGraphJobID();
    }

    if (_graph->isScheduled())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph has been already scheduled.");
        return JobGraphJobID();
    }

    if (!_graph->m_entryJob || !_graph->m_exitJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph boundary jobs are invalid.");
        return JobGraphJobID();
    }

    if (_graph->m_entryJob->m_parentCounter.load() != 0 || !_graph->m_exitJob->m_dependencyJobs.empty())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph has external dependencies.");
        return JobGraphJobID();
    }

    for (const GraphBuildingContext::Node& node : m_buildingContext.m_nodes)
    {
        if (node.m_type == GraphBuildingContext::NodeType::JobGraph && node.m_jobGraph.get() == _graph.get())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph has been already added to this builder.");
            return JobGraphJobID();
        }
    }

    return m_buildingContext.addJobGraph(_graph);
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphBefore(const JobGraphReference& _graph, JobGraphJobID _childJobID)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphBefore(const JobGraphReference& _graph, const JobGraphJobIDCollection& _childJobIDs)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphAfter(const JobGraphReference& _graph, JobGraphJobID _parentJobID)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphAfter(const JobGraphReference& _graph, const JobGraphJobIDCollection& _parentJobIDs)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependenciesAfter(_parentJobIDs, jobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphBetween(const JobGraphReference& _graph, JobGraphJobID _parentJobID, JobGraphJobID _childJobID)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependency(_parentJobID, jobID);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphBetween(
    const JobGraphReference& _graph,
    const JobGraphJobIDCollection& _parentJobIDs,
    const JobGraphJobIDCollection& _childJobIDs)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    if (!jobID.isValid())
    {
        return jobID;
    }

    makeDependenciesAfter(_parentJobIDs, jobID);
    makeDependenciesBefore(jobID, _childJobIDs);

    return jobID;
}

void ego::JobGraphBuilder::makeDependency(JobGraphJobID _parentJobID, JobGraphJobID _childJobID)
{
    if (!m_buildingContext.isValid(_parentJobID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Parent job index is invalid.");
        return;
    }

    if (!m_buildingContext.isValid(_childJobID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Child job index is invalid.");
        return;
    }

    if (_parentJobID == _childJobID)
    {
        EGO_ASSERT_FAIL_MESSAGE("Child and parent job ids are same.");
        return;
    }

    for (const GraphBuildingContext::Dependency& dependency : m_buildingContext.m_dependencies)
    {
        if (dependency.m_parentJobID == _parentJobID && dependency.m_childJobID == _childJobID)
        {
            return;
        }
    }

    m_buildingContext.m_dependencies.push_back(GraphBuildingContext::Dependency{_parentJobID, _childJobID});
}

void ego::JobGraphBuilder::makeDependenciesBefore(JobGraphJobID _parentJobID, const JobGraphJobIDCollection& _childJobIDs)
{
    for (const JobGraphJobID childJobID : _childJobIDs)
    {
        makeDependency(_parentJobID, childJobID);
    }
}

void ego::JobGraphBuilder::makeDependenciesAfter(const JobGraphJobIDCollection& _parentJobIDs, JobGraphJobID _childJobID)
{
    for (const JobGraphJobID parentJobID : _parentJobIDs)
    {
        makeDependency(parentJobID, _childJobID);
    }
}

ego::JobGraphReference ego::JobGraphBuilder::getGraph()
{
    if (m_buildingContext.m_nodes.empty())
    {
        return nullptr;
    }

    for (const GraphBuildingContext::Node& node : m_buildingContext.m_nodes)
    {
        if (node.m_type == GraphBuildingContext::NodeType::Job && !node.m_job)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph node job is invalid.");
            m_buildingContext.clear();
            return nullptr;
        }

        if (node.m_type == GraphBuildingContext::NodeType::JobGraph && !node.m_jobGraph)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph node graph is invalid.");
            m_buildingContext.clear();
            return nullptr;
        }

        if (node.m_type != GraphBuildingContext::NodeType::JobGraph)
        {
            continue;
        }

        if (node.m_jobGraph->isScheduled())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph node graph has been already scheduled.");
            m_buildingContext.clear();
            return nullptr;
        }

        if (!node.m_jobGraph->m_entryJob || !node.m_jobGraph->m_exitJob)
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph node graph boundary jobs are invalid.");
            m_buildingContext.clear();
            return nullptr;
        }

        if (node.m_jobGraph->m_entryJob->m_parentCounter.load() != 0 || !node.m_jobGraph->m_exitJob->m_dependencyJobs.empty())
        {
            EGO_ASSERT_FAIL_MESSAGE("Job graph node graph has external dependencies.");
            m_buildingContext.clear();
            return nullptr;
        }
    }

    std::vector<uint32_t> parentCounters(m_buildingContext.m_nodes.size(), 0);
    std::vector<std::vector<uint32_t>> childJobs(m_buildingContext.m_nodes.size());

    for (const GraphBuildingContext::Dependency& dependency : m_buildingContext.m_dependencies)
    {
        if (!m_buildingContext.isValid(dependency.m_parentJobID) || !m_buildingContext.isValid(dependency.m_childJobID))
        {
            EGO_ASSERT_FAIL_MESSAGE("Graph dependency data is invalid.");
            m_buildingContext.clear();
            return nullptr;
        }

        childJobs[dependency.m_parentJobID.getIndex()].push_back(dependency.m_childJobID.getIndex());
        ++parentCounters[dependency.m_childJobID.getIndex()];
    }

    std::queue<uint32_t> readyValidationJobs;
    for (uint32_t jobIndex = 0; jobIndex < parentCounters.size(); ++jobIndex)
    {
        if (parentCounters[jobIndex] == 0)
        {
            readyValidationJobs.push(jobIndex);
        }
    }

    uint32_t validatedJobCount = 0;
    std::vector<uint32_t> validationParentCounters = parentCounters;
    while (!readyValidationJobs.empty())
    {
        const uint32_t jobIndex = readyValidationJobs.front();
        readyValidationJobs.pop();
        ++validatedJobCount;

        for (const uint32_t childJobIndex : childJobs[jobIndex])
        {
            if (--validationParentCounters[childJobIndex] == 0)
            {
                readyValidationJobs.push(childJobIndex);
            }
        }
    }

    if (validatedJobCount != m_buildingContext.m_nodes.size())
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph contains dependency cycle.");
        m_buildingContext.clear();
        return nullptr;
    }

    struct NodeFragment final
    {
        JobGraph::DependencyJobReference m_entryJob;
        JobGraph::DependencyJobReference m_exitJob;
    };

    const JobGraph::GraphJobDataReference graphData = new JobGraph::GraphJobData(getDbgName());
    const JobGraph::DependencyJobReference graphEntryJob = new JobGraph::BarrierDependencyJob(graphData, JobGraph::BarrierDependencyJob::Type::Begin);
    const JobGraph::DependencyJobReference graphExitJob = new JobGraph::BarrierDependencyJob(graphData, JobGraph::BarrierDependencyJob::Type::End);

    std::vector<NodeFragment> nodeFragments;
    nodeFragments.reserve(m_buildingContext.m_nodes.size());

    JobGraph::DependencyJobCollection graphJobs;
    graphJobs.reserve(m_buildingContext.m_nodes.size() + 2);
    graphJobs.push_back(graphEntryJob);
    graphJobs.push_back(graphExitJob);

    JobGraph::JobGraphCollection nestedGraphs;

    for (const GraphBuildingContext::Node& node : m_buildingContext.m_nodes)
    {
        if (node.m_type == GraphBuildingContext::NodeType::Job)
        {
            const JobGraph::DependencyJobReference graphJob = new JobGraph::JobDependencyJob(node.m_job, graphData);
            graphJobs.push_back(graphJob);
            nodeFragments.push_back(NodeFragment{graphJob, graphJob});
        }
        else
        {
            nestedGraphs.push_back(node.m_jobGraph);
            nodeFragments.push_back(NodeFragment{node.m_jobGraph->m_entryJob, node.m_jobGraph->m_exitJob});
        }
    }

    for (const GraphBuildingContext::Dependency& dependency : m_buildingContext.m_dependencies)
    {
        const uint32_t parentJobIndex = dependency.m_parentJobID.getIndex();
        const uint32_t childJobIndex = dependency.m_childJobID.getIndex();

        const NodeFragment& parentFragment = nodeFragments[parentJobIndex];
        const NodeFragment& childFragment = nodeFragments[childJobIndex];

        parentFragment.m_exitJob->m_dependencyJobs.push_back(childFragment.m_entryJob);
        ++childFragment.m_entryJob->m_parentCounter;
    }

    for (uint32_t jobIndex = 0; jobIndex < nodeFragments.size(); ++jobIndex)
    {
        if (parentCounters[jobIndex] == 0)
        {
            graphEntryJob->m_dependencyJobs.push_back(nodeFragments[jobIndex].m_entryJob);
            ++nodeFragments[jobIndex].m_entryJob->m_parentCounter;
        }

        if (childJobs[jobIndex].empty())
        {
            nodeFragments[jobIndex].m_exitJob->m_dependencyJobs.push_back(graphExitJob);
            ++graphExitJob->m_parentCounter;
        }
    }

    graphData->m_jobCounter = static_cast<uint32_t>(graphJobs.size());

    m_buildingContext.clear();

    if (graphEntryJob->m_dependencyJobs.empty() || graphExitJob->m_parentCounter.load() == 0)
    {
        return nullptr;
    }

    return new JobGraph(graphEntryJob, graphExitJob, std::move(nestedGraphs), graphData);
}
