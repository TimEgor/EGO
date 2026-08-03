#include "DiagnosticSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Assert/AssertController.h"
#include "EgoCore/Log/LogController.h"
#include "EgoCore/Profile/ProfileController.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoCore/Subsystem/SubsystemRegistry.h"

ego::DiagnosticSubsystem::~DiagnosticSubsystem()
{
    release();
}

bool ego::DiagnosticSubsystem::init()
{
    release();

    m_assertController = MakePointer<AssertController>();
    EGO_CHECK_INITIALIZATION(m_assertController);

    m_loggerController = MakePointer<log::LoggerController>();
    EGO_CHECK_INITIALIZATION(m_loggerController);
    EGO_CHECK_INITIALIZATION(m_loggerController->init());

    m_profilerController = MakePointer<profile::ProfilerController>();
    EGO_CHECK_INITIALIZATION(m_profilerController);

    return true;
}

void ego::DiagnosticSubsystem::release()
{
    m_profilerController = nullptr;
    m_loggerController = nullptr;
    m_assertController = nullptr;
}

ego::AssertControllerPointer ego::DiagnosticSubsystem::getAssertController() const
{
    return m_assertController;
}

ego::log::LoggerControllerPointer ego::DiagnosticSubsystem::getLoggerController() const
{
    return m_loggerController;
}

ego::profile::ProfilerControllerPointer ego::DiagnosticSubsystem::getProfilerController() const
{
    return m_profilerController;
}

ego::DiagnosticSubsystemPointer ego::GetDiagnosticSubsystemPointer()
{
    return subsystem::FindSubsystem<DiagnosticSubsystem>();
}

ego::DiagnosticSubsystem& ego::GetDiagnosticSubsystem()
{
    const DiagnosticSubsystemPointer diagnosticSubsystem = GetDiagnosticSubsystemPointer();
    EGO_ASSERT(diagnosticSubsystem);

    return *diagnosticSubsystem;
}

ego::AssertControllerPointer ego::GetAssertController()
{
    const DiagnosticSubsystemPointer diagnosticSubsystem = GetDiagnosticSubsystemPointer();
    return diagnosticSubsystem ? diagnosticSubsystem->getAssertController() : nullptr;
}

ego::log::LoggerControllerPointer ego::GetLoggerController()
{
    const DiagnosticSubsystemPointer diagnosticSubsystem = GetDiagnosticSubsystemPointer();
    return diagnosticSubsystem ? diagnosticSubsystem->getLoggerController() : nullptr;
}

ego::profile::ProfilerControllerPointer ego::GetProfilerController()
{
    const DiagnosticSubsystemPointer diagnosticSubsystem = GetDiagnosticSubsystemPointer();
    return diagnosticSubsystem ? diagnosticSubsystem->getProfilerController() : nullptr;
}
