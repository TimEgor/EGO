#include "FrameLogic.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

bool ego::engine::FrameLogic::init(
    const JobDescriptor& _renderFrameJobDescriptor,
    const JobDescriptor& _presentFrameJobDescriptor,
    const JobDescriptor& _prepareRenderFrameJobDescriptor)
{
    release();
    m_jobGraphDescriptorBuilder.setDbgName("Frame logic");

    m_frameJobs.m_beginJobID = m_jobGraphDescriptorBuilder.addJob(ego::CreateEmptyJobDescriptor("Frame begin"));
    EGO_CHECK_RETURN_FALSE(m_frameJobs.m_beginJobID.isValid());

    m_renderJobs.m_beginJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Render begin"), m_frameJobs.m_beginJobID);
    EGO_CHECK_RETURN_FALSE(m_renderJobs.m_beginJobID.isValid());

    m_renderJobs.m_renderJobID = m_jobGraphDescriptorBuilder.addJobAfter(_renderFrameJobDescriptor, m_renderJobs.m_beginJobID);
    EGO_CHECK_RETURN_FALSE(m_renderJobs.m_renderJobID.isValid());

    m_renderJobs.m_presentJobID = m_jobGraphDescriptorBuilder.addJobAfter(_presentFrameJobDescriptor, m_renderJobs.m_renderJobID);
    EGO_CHECK_RETURN_FALSE(m_renderJobs.m_presentJobID.isValid());

    m_renderJobs.m_endJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Render end"), m_renderJobs.m_presentJobID);
    EGO_CHECK_RETURN_FALSE(m_renderJobs.m_endJobID.isValid());

    m_frameLogicJobs.m_beginJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Frame logic begin"), m_frameJobs.m_beginJobID);
    EGO_CHECK_RETURN_FALSE(m_frameLogicJobs.m_beginJobID.isValid());

    m_frameLogicJobs.m_endJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Frame logic end"), m_frameLogicJobs.m_beginJobID);
    EGO_CHECK_RETURN_FALSE(m_frameLogicJobs.m_endJobID.isValid());

    m_prepareRenderJobs.m_beginJobID = m_jobGraphDescriptorBuilder.addJobAfter(
        ego::CreateEmptyJobDescriptor("Prepare render begin"),
        JobDescriptorIDCollection{m_renderJobs.m_endJobID, m_frameLogicJobs.m_endJobID});
    EGO_CHECK_RETURN_FALSE(m_prepareRenderJobs.m_beginJobID.isValid());

    m_prepareRenderJobs.m_prepareJobID = m_jobGraphDescriptorBuilder.addJobAfter(_prepareRenderFrameJobDescriptor, m_prepareRenderJobs.m_beginJobID);
    EGO_CHECK_RETURN_FALSE(m_prepareRenderJobs.m_prepareJobID.isValid());

    m_prepareRenderJobs.m_endJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Prepare render end"), m_prepareRenderJobs.m_prepareJobID);
    EGO_CHECK_RETURN_FALSE(m_prepareRenderJobs.m_endJobID.isValid());

    m_frameJobs.m_endJobID = m_jobGraphDescriptorBuilder.addJobAfter(ego::CreateEmptyJobDescriptor("Frame end"), m_prepareRenderJobs.m_endJobID);
    EGO_CHECK_RETURN_FALSE(m_frameJobs.m_endJobID.isValid());

    m_isInitialized = true;
    return true;
}

void ego::engine::FrameLogic::release()
{
    m_frameJobs = FrameJobs();
    m_renderJobs = RenderJobs();
    m_frameLogicJobs = FrameLogicJobs();
    m_prepareRenderJobs = PrepareRenderJobs();
    m_jobGraphDescriptorBuilder.clear();
    m_isInitialized = false;
}

ego::JobDescriptorID ego::engine::FrameLogic::addJob(const JobDescriptor& _descriptor)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJob(_descriptor);
}

bool ego::engine::FrameLogic::removeJob(JobDescriptorID _jobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.removeJob(_jobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobBefore(const JobDescriptor& _descriptor, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobBefore(_descriptor, _childJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobBefore(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _childJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobBefore(_descriptor, _childJobIDs);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobAfter(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobAfter(_descriptor, _parentJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobAfter(const JobDescriptor& _descriptor, const JobDescriptorIDCollection& _parentJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobAfter(_descriptor, _parentJobIDs);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobBetween(const JobDescriptor& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobBetween(_descriptor, _parentJobID, _childJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobBetween(
    const JobDescriptor& _descriptor,
    const JobDescriptorIDCollection& _parentJobIDs,
    const JobDescriptorIDCollection& _childJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobBetween(_descriptor, _parentJobIDs, _childJobIDs);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraph(const JobGraphDescriptorPointer& _descriptor)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraph(_descriptor);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphBefore(_descriptor, _childJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphBefore(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _childJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphBefore(_descriptor, _childJobIDs);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphAfter(_descriptor, _parentJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphAfter(const JobGraphDescriptorPointer& _descriptor, const JobDescriptorIDCollection& _parentJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphAfter(_descriptor, _parentJobIDs);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphBetween(const JobGraphDescriptorPointer& _descriptor, JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphBetween(_descriptor, _parentJobID, _childJobID);
}

ego::JobDescriptorID ego::engine::FrameLogic::addJobGraphBetween(
    const JobGraphDescriptorPointer& _descriptor,
    const JobDescriptorIDCollection& _parentJobIDs,
    const JobDescriptorIDCollection& _childJobIDs)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphBetween(_descriptor, _parentJobIDs, _childJobIDs);
}

void ego::engine::FrameLogic::makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    m_jobGraphDescriptorBuilder.makeDependency(_parentJobID, _childJobID);
}

const ego::engine::FrameLogic::FrameJobs& ego::engine::FrameLogic::getFrameJobs() const
{
    return m_frameJobs;
}

ego::JobDescriptorID ego::engine::FrameLogic::getFrameBeginJobID() const
{
    return m_frameJobs.m_beginJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getFrameEndJobID() const
{
    return m_frameJobs.m_endJobID;
}

const ego::engine::FrameLogic::RenderJobs& ego::engine::FrameLogic::getRenderJobs() const
{
    return m_renderJobs;
}

ego::JobDescriptorID ego::engine::FrameLogic::getRenderBeginJobID() const
{
    return m_renderJobs.m_beginJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getRenderJobID() const
{
    return m_renderJobs.m_renderJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getPresentJobID() const
{
    return m_renderJobs.m_presentJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getRenderEndJobID() const
{
    return m_renderJobs.m_endJobID;
}

const ego::engine::FrameLogic::FrameLogicJobs& ego::engine::FrameLogic::getFrameLogicJobs() const
{
    return m_frameLogicJobs;
}

ego::JobDescriptorID ego::engine::FrameLogic::getFrameLogicBeginJobID() const
{
    return m_frameLogicJobs.m_beginJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getFrameLogicEndJobID() const
{
    return m_frameLogicJobs.m_endJobID;
}

const ego::engine::FrameLogic::PrepareRenderJobs& ego::engine::FrameLogic::getPrepareRenderJobs() const
{
    return m_prepareRenderJobs;
}

ego::JobDescriptorID ego::engine::FrameLogic::getPrepareRenderBeginJobID() const
{
    return m_prepareRenderJobs.m_beginJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getPrepareRenderJobID() const
{
    return m_prepareRenderJobs.m_prepareJobID;
}

ego::JobDescriptorID ego::engine::FrameLogic::getPrepareRenderEndJobID() const
{
    return m_prepareRenderJobs.m_endJobID;
}

ego::JobGraphPointer ego::engine::FrameLogic::createJobGraph() const
{
    EGO_CHECK_RETURN_NULL(m_isInitialized);

    const JobGraphDescriptorPointer descriptor = m_jobGraphDescriptorBuilder.build();
    EGO_CHECK_RETURN_NULL(descriptor);

    return descriptor->createJobGraph();
}

bool ego::engine::FrameLogic::isInitialized() const
{
    return m_isInitialized;
}
