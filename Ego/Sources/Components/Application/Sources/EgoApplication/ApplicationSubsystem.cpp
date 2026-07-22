#include "ApplicationSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

bool ego::application::ApplicationSubsystem::init(const ApplicationPointer& _application)
{
    EGO_CHECK_INITIALIZATION(m_application.isExpired());
    EGO_CHECK_INITIALIZATION(_application);

    m_application = _application;
    return true;
}

void ego::application::ApplicationSubsystem::release()
{
    m_application.reset();
}

ego::application::ApplicationPointer ego::application::ApplicationSubsystem::getApplicationPointer() const
{
    return m_application.lock();
}

ego::application::Application& ego::application::ApplicationSubsystem::getApplication() const
{
    const ApplicationPointer application = getApplicationPointer();
    EGO_ASSERT(application);

    return *application;
}

ego::application::ApplicationSubsystemPointer ego::application::GetApplicationSubsystemPointer()
{
    return subsystem::FindSubsystem<ApplicationSubsystem>();
}

ego::application::ApplicationSubsystem& ego::application::GetApplicationSubsystem()
{
    const ApplicationSubsystemPointer applicationSubsystem = GetApplicationSubsystemPointer();
    EGO_ASSERT(applicationSubsystem);

    return *applicationSubsystem;
}

ego::application::ApplicationPointer ego::application::GetApplicationPointer()
{
    const ApplicationSubsystemPointer applicationSubsystem = GetApplicationSubsystemPointer();
    return applicationSubsystem ? applicationSubsystem->getApplicationPointer() : nullptr;
}

ego::application::Application& ego::application::GetApplication()
{
    const ApplicationPointer application = GetApplicationPointer();
    EGO_ASSERT(application);

    return *application;
}
