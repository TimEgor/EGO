#include "JobGraph.h"

#include "EgoCore/UtilsMacros.h"
#include "EgoCore/Assert/AssertCore.h"

#include "JobController.h"

ego::JobGraph::GraphJobData::JobGraphEvent::JobGraphEvent(GraphJobData* _graph)
    : m_graph(_graph)
{}

void ego::JobGraph::GraphJobData::JobGraphEvent::wait()
{
    std::unique_lock locker(m_graph->m_mutex);
    m_notifier.wait(locker, [this]() { return m_graph->m_jobCounter == 0; });
}

void ego::JobGraph::GraphJobData::JobGraphEvent::set()
{
    m_notifier.notify_all();
}

ego::JobGraph::GraphJobData::~GraphJobData()
{
    EGO_SAFE_DESTROY(m_completionEvent);
}

void ego::JobGraph::GraphJobData::completeJob()
{
    if (m_jobCounter-- == 1)
    {
        std::lock_guard locker(m_mutex);

        for (const auto& callback : m_completionCallback)
        {
            callback();
        }

        if (m_completionEvent)
        {
            m_completionEvent->set();
        }
    }
}

void ego::JobGraph::GraphJobData::addCompletionCallback(const CompletionCallback& _callback)
{
    if (_callback)
    {
        std::lock_guard locker(m_mutex);
        if (m_jobCounter > 0)
        {
            m_completionCallback.push_back(_callback);
        }
    }
    else
    {
        EGO_ASSERT_FAIL_MESSAGE("Graph job completion callback is null.");
    }
}

void ego::JobGraph::GraphJobData::setExecutionContext(const JobControllerWeakPointer& _controller)
{
    std::lock_guard locker(m_mutex);
    m_jobController = _controller;
}

void ego::JobGraph::GraphJobData::wait()
{
    if (isFinished())
    {
        return;
    }

    {
        std::lock_guard locker(m_mutex);
        if (!m_completionEvent)
        {
            m_completionEvent = new JobGraphEvent(this);
        }
    }

    m_completionEvent->wait();
}

bool ego::JobGraph::GraphJobData::isFinished() const
{
    return m_jobCounter == 0;
}

ego::JobGraph::DependencyJobBase::DependencyJobBase(const GraphJobDataReference& _graphData, const char* _dbgName)
    : Job(_dbgName),
      m_graphData(_graphData) {}

void ego::JobGraph::DependencyJobBase::removeChildDependencies()
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

void ego::JobGraph::DependencyJobBase::operate()
{
    removeChildDependencies();

    m_graphData->completeJob();
}

bool ego::JobGraph::DependencyJobBase::removeParentDependency()
{
    return m_parentCounter-- == 1;
}

ego::JobGraph::DependencyJob::DependencyJob(const JobReference& _job, const GraphJobDataReference& _graphData)
    : DependencyJobBase(_graphData, "GraphJob"),
      m_job(_job) {}

void ego::JobGraph::DependencyJob::operate()
{
    m_job->execute();

    DependencyJobBase::operate();
}

ego::JobGraph::PostGraphDependencyJob::PostGraphDependencyJob(const GraphJobDataReference& _graphData)
    : DependencyJobBase(_graphData, "PostGraphJob") {}

ego::JobGraph::PreGraphDependencyJob::PreGraphDependencyJob(
    const GraphJobDataReference& _graphData,
    const PostGraphDependencyJobReference& _postGraphJob,
    const JobGraphReference& _graph
)
    : DependencyJobBase(_graphData, "PreGraphJob"),
      m_graph(_graph)
{
    GraphJobDataReference dependencyGraphData = m_graph->getGraphData();
    dependencyGraphData->addCompletionCallback(
        [graphData = m_graphData, _postGraphJob]()
        {
            JobControllerPointer jobController = graphData->getJobController();
            EGO_ASSERT(jobController);
            jobController->addJob(_postGraphJob);
        }
    );
}

void ego::JobGraph::PreGraphDependencyJob::operate()
{
    JobControllerPointer jobController = getJobController();
    EGO_ASSERT(jobController);
    jobController->addJobGraph(m_graph);

    DependencyJobBase::operate();
}

ego::JobGraph::JobGraph(JobCollection&& _baseJobs, GraphJobDataReference _data)
    : m_baseJobs(std::move(_baseJobs)),
      m_graphData(_data)
{}

ego::JobGraph::JobCollection&& ego::JobGraph::getBaseJobs()
{
    return std::move(m_baseJobs);
}

ego::JobGraph::GraphJobDataReference ego::JobGraph::getGraphData() const
{
    return m_graphData;
}

void ego::JobGraph::setExecutionContext(const JobControllerWeakPointer& _jobController)
{
    m_graphData->setExecutionContext(_jobController);
}

void ego::JobGraph::wait()
{
    m_graphData->wait();
}

bool ego::JobGraph::isFinished() const
{
    return m_graphData->isFinished();
}

ego::JobGraphBuilder::GraphBuildingContext::GraphBuildingContext()
{
    m_graphData = new JobGraph::GraphJobData();
}

ego::JobGraphBuilder::JobGraphJobID::JobGraphJobID(uint32_t _id)
    : m_value(_id) {}

ego::JobGraphBuilder::JobGraphJobID::JobGraphJobID(bool _isGraph, uint32_t _index)
    : m_isGraphJob(_isGraph),
      m_index(_index) {}

bool ego::JobGraphBuilder::JobGraphJobID::operator==(JobGraphJobID _id) const
{
    return _id.m_value == m_value;
}

ego::JobGraphBuilder::JobGraphJobID::operator uint32_t() const
{
    return m_value;
}

ego::JobGraphBuilder::~JobGraphBuilder()
{
    releaseContext();
}

ego::JobGraph::DependencyJobReference* ego::JobGraphBuilder::GraphBuildingContext::getJob(JobGraphJobID _jobID)
{
    if (_jobID.m_isGraphJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job is graph typed.");
        return nullptr;
    }

    JobGraph::DependencyJobReference* job = nullptr;

    if (m_jobs.size() > _jobID.m_index)
    {
        job = &m_jobs[_jobID.m_index];
    }
    else
    {
        EGO_ASSERT_FAIL_MESSAGE("Job index is invalid.");
        return nullptr;
    }

    return job;
}

ego::JobGraphBuilder::GraphBuildingContext::GraphJobInfo ego::JobGraphBuilder::GraphBuildingContext::getGraphJob(
    JobGraphJobID _jobID,
    bool _isParent
)
{
    if (!_jobID.m_isGraphJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job isn't graph typed.");
        return GraphJobInfo();
    }

    GraphJobInfo jobInfo;

    if (m_graphNodes.size() > _jobID.m_index)
    {
        const GraphNode& graphNode = m_graphNodes[_jobID.m_index];

        const JobGraphJobID graphJobID = _isParent ? graphNode.m_postJobID : graphNode.m_preJobID;
        jobInfo.m_job = getJob(graphJobID);
        jobInfo.m_jobID = graphJobID;
    }
    else
    {
        EGO_ASSERT_FAIL_MESSAGE("Graph job index is invalid.");
    }

    return jobInfo;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::GraphBuildingContext::addDependencyJob(
    const JobGraph::DependencyJobReference& _job,
    bool _addToCollection
)
{
    EGO_ASSERT(_job);

    const JobGraphJobID jobId(false, m_lastJobIndex);

    m_jobs.emplace_back(_job);

    if (_addToCollection)
    {
        m_dependencyCounterCollection[jobId] = 0;
    }

    ++m_lastJobIndex;
    ++m_graphData->m_jobCounter;

    return jobId;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::GraphBuildingContext::addGraphNode(
    JobGraphJobID _preJobID,
    JobGraphJobID _postJobID
)
{
    const JobGraphJobID jobId(true, m_lastGraphIndex);

    m_graphNodes.emplace_back(GraphNode{_preJobID, _postJobID});

    ++m_lastGraphIndex;

    return jobId;
}

void ego::JobGraphBuilder::initContext()
{
    if (!m_buildingContext)
    {
        m_buildingContext = new GraphBuildingContext();
    }
}

void ego::JobGraphBuilder::releaseContext()
{
    EGO_SAFE_DESTROY(m_buildingContext);
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJob(const JobReference& _job)
{
    if (!_job)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job is invalid.");
        return JobGraphJobID(false, InvalidJobGraphJobIndex);
    }

    initContext();

    const JobGraph::DependencyJobReference graphJob(new JobGraph::DependencyJob(_job, m_buildingContext->m_graphData));

    return m_buildingContext->addDependencyJob(graphJob);
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobBefore(
    const JobReference& _job,
    JobGraphJobID _childJobID
)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobAfter(
    const JobReference& _job,
    JobGraphJobID _parentJobID
)
{
    const JobGraphJobID jobID = addJob(_job);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraph(const JobGraphReference& _graph)
{
    initContext();

    const JobGraph::PostGraphDependencyJobReference postGraphJob(new JobGraph::PostGraphDependencyJob(
        m_buildingContext->m_graphData
    ));
    const JobGraphJobID postGraphJobID = m_buildingContext->addDependencyJob(postGraphJob, false);

    const JobGraph::DependencyJobReference preGraphJob(new JobGraph::PreGraphDependencyJob(
        m_buildingContext->m_graphData,
        postGraphJob,
        _graph
    ));
    const JobGraphJobID preGraphJobID = m_buildingContext->addDependencyJob(preGraphJob);

    return m_buildingContext->addGraphNode(preGraphJobID, postGraphJobID);
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphBefore(
    const JobGraphReference& _graph,
    JobGraphJobID _childJobID
)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependency(jobID, _childJobID);

    return jobID;
}

ego::JobGraphBuilder::JobGraphJobID ego::JobGraphBuilder::addJobGraphAfter(
    const JobGraphReference& _graph,
    JobGraphJobID _parentJobID
)
{
    const JobGraphJobID jobID = addJobGraph(_graph);
    makeDependency(_parentJobID, jobID);

    return jobID;
}

void ego::JobGraphBuilder::makeDependency(JobGraphJobID _parentJobID, JobGraphJobID _childJobID)
{
    if (!m_buildingContext)
    {
        EGO_ASSERT_FAIL_MESSAGE("Job graph building context hasn't been inited.");
        return;
    }

    if (_parentJobID == _childJobID)
    {
        EGO_ASSERT_FAIL_MESSAGE("Child and parent job ids are same.");
        return;
    }

    if (_parentJobID.m_index == InvalidJobGraphJobIndex)
    {
        EGO_ASSERT_FAIL_MESSAGE("Parent job index is invalid.");
        return;
    }

    if (_childJobID.m_index == InvalidJobGraphJobIndex)
    {
        EGO_ASSERT_FAIL_MESSAGE("Child job index is invalid.");
        return;
    }

    JobGraph::DependencyJobReference* parentJob;
    if (_parentJobID.m_isGraphJob)
    {
        parentJob = m_buildingContext->getGraphJob(_parentJobID, true).m_job;
    }
    else
    {
        parentJob = m_buildingContext->getJob(_parentJobID);
    }

    if (!parentJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Parent job hasn't been found.");
        return;
    }

    JobGraphJobID dependencyChildJobID = _childJobID;
    JobGraph::DependencyJobReference* childJob;
    if (_childJobID.m_isGraphJob)
    {
        GraphBuildingContext::GraphJobInfo info = m_buildingContext->getGraphJob(_childJobID, false);
        childJob = info.m_job;
        dependencyChildJobID = info.m_jobID;
    }
    else
    {
        childJob = m_buildingContext->getJob(_childJobID);
    }

    if (!childJob)
    {
        EGO_ASSERT_FAIL_MESSAGE("Child job hasn't been found.");
        return;
    }

    (*parentJob)->m_dependencyJobs.push_back(*childJob);
    ++(*childJob)->m_parentCounter;
    ++m_buildingContext->m_dependencyCounterCollection.at(dependencyChildJobID);
}

ego::JobGraphReference ego::JobGraphBuilder::getGraph()
{
    if (!m_buildingContext)
    {
        return nullptr;
    }

    if (m_buildingContext->m_jobs.empty())
    {
        releaseContext();
        return nullptr;
    }

    std::vector<JobReference> m_baseJobs;

    for (const auto& jobDependencyCounter : m_buildingContext->m_dependencyCounterCollection)
    {
        if (jobDependencyCounter.second == 0)
        {
            const JobGraph::DependencyJobReference* job = m_buildingContext->getJob(jobDependencyCounter.first);
            if (!job)
            {
                EGO_ASSERT_FAIL_MESSAGE("Graph job structure is invalid.");
                continue;
            }

            m_baseJobs.push_back(*job);
        }
    }

    const JobGraph::GraphJobDataReference graphData = m_buildingContext->m_graphData;

    releaseContext();

    if (m_baseJobs.empty())
    {
        return nullptr;
    }

    return new JobGraph(std::move(m_baseJobs), graphData);
}
