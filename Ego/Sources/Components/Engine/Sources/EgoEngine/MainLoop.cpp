#include "MainLoop.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

bool ego::engine::MainLoop::init(const JobDescriptor& _renderJobDescriptor)
{
    release();

    m_renderJobID = m_jobGraphDescriptorBuilder.addJob(_renderJobDescriptor);
    EGO_CHECK_RETURN_FALSE(m_renderJobID.isValid());

    m_isInitialized = true;
    return true;
}

void ego::engine::MainLoop::release()
{
    m_renderJobID = JobDescriptorID();
    m_jobGraphDescriptorBuilder.clear();
    m_isInitialized = false;
}

ego::JobDescriptorID ego::engine::MainLoop::addJob(const JobDescriptor& _descriptor)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJob(_descriptor);
}

ego::JobDescriptorID ego::engine::MainLoop::addJobBefore(
    const JobDescriptor& _descriptor,
    JobDescriptorID _childJobID
)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobBefore(_descriptor, _childJobID);
}

ego::JobDescriptorID ego::engine::MainLoop::addJobAfter(
    const JobDescriptor& _descriptor,
    JobDescriptorID _parentJobID
)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobAfter(_descriptor, _parentJobID);
}

ego::JobDescriptorID ego::engine::MainLoop::addJobGraph(const JobGraphDescriptorPointer& _descriptor)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraph(_descriptor);
}

ego::JobDescriptorID ego::engine::MainLoop::addJobGraphBefore(
    const JobGraphDescriptorPointer& _descriptor,
    JobDescriptorID _childJobID
)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphBefore(_descriptor, _childJobID);
}

ego::JobDescriptorID ego::engine::MainLoop::addJobGraphAfter(
    const JobGraphDescriptorPointer& _descriptor,
    JobDescriptorID _parentJobID
)
{
    EGO_ASSERT(m_isInitialized);
    return m_jobGraphDescriptorBuilder.addJobGraphAfter(_descriptor, _parentJobID);
}

void ego::engine::MainLoop::makeDependency(JobDescriptorID _parentJobID, JobDescriptorID _childJobID)
{
    EGO_ASSERT(m_isInitialized);
    m_jobGraphDescriptorBuilder.makeDependency(_parentJobID, _childJobID);
}

ego::JobDescriptorID ego::engine::MainLoop::getRenderJobID() const
{
    return m_renderJobID;
}

ego::JobGraphReference ego::engine::MainLoop::createJobGraph() const
{
    EGO_CHECK_RETURN_NULL(m_isInitialized);

    const JobGraphDescriptorPointer descriptor = m_jobGraphDescriptorBuilder.build();
    EGO_CHECK_RETURN_NULL(descriptor);

    return descriptor->createJobGraph();
}

bool ego::engine::MainLoop::isInitialized() const
{
    return m_isInitialized;
}
