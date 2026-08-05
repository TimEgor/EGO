#include "Application.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

ego::application::Application::~Application()
{
    release();
}

bool ego::application::Application::init()
{
    EGO_ASSERT(!m_subsystemRegistry);
    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);
    EGO_CHECK_INITIALIZATION_ASSERT_MESSAGE(initSubsystemRegistry(), "Failed to initialize the application subsystem registry.");

    return true;
}

void ego::application::Application::release()
{
    releaseSubsystemRegistry();
}

bool ego::application::Application::initSubsystemRegistry()
{
    EGO_CHECK_RETURN_FALSE(!m_subsystemRegistry);

    m_subsystemRegistry = MakePointer<subsystem::SubsystemRegistry>();
    EGO_CHECK_RETURN_FALSE(m_subsystemRegistry);

    if (!subsystem::SubsystemLocator::GetInstance().bind(m_subsystemRegistry))
    {
        m_subsystemRegistry = nullptr;

        return false;
    }

    return true;
}

void ego::application::Application::releaseSubsystemRegistry()
{
    if (!m_subsystemRegistry)
    {
        return;
    }

    subsystem::SubsystemLocator& subsystemLocator = subsystem::SubsystemLocator::GetInstance();
    if (subsystemLocator.getRegistryPointer().get() == m_subsystemRegistry.get())
    {
        subsystemLocator.unbind();
    }

    m_subsystemRegistry = nullptr;
}
