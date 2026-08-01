#include "ApplicationSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

bool ego::application::ApplicationSubsystem::init(const Application::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!m_application);

    m_application = MakePointer<Application>();
    EGO_CHECK_INITIALIZATION(m_application && m_application->init(_initData, sharedFromThis()));

    return true;
}

void ego::application::ApplicationSubsystem::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_application);
}

ego::application::ApplicationPointer ego::application::ApplicationSubsystem::getApplicationPointer() const
{
    return m_application;
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
