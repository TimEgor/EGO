#include "DiagnosticContext.h"

#include "ContextStack.h"
#include "EgoCore/Assert/AssertCore.h"

void ego::context::DiagnosticContext::release()
{
    m_profilerController = nullptr;
    m_logger = nullptr;
    m_assertGenerator = nullptr;
}

void ego::context::DiagnosticContext::setAssertGenerator(const AssertGeneratorPointer& _assertGenerator)
{
    if (m_assertGenerator)
    {
        EGO_ASSERT_FAIL_MESSAGE("Assert generator has been already set.");
        return;
    }

    m_assertGenerator = _assertGenerator;
}

ego::AssertGeneratorPointer ego::context::DiagnosticContext::getAssertGenerator() const
{
    return m_assertGenerator;
}

void ego::context::DiagnosticContext::setLogger(const log::LoggerPointer& _logger)
{
    m_logger = _logger;
}

ego::log::LoggerPointer ego::context::DiagnosticContext::getLogger() const
{
    return m_logger;
}

void ego::context::DiagnosticContext::setProfilerController(const profile::ProfilerControllerPointer& _profilerController)
{
    EGO_CHECK_RETURN(_profilerController);
    m_profilerController = _profilerController;
}

ego::profile::ProfilerControllerPointer ego::context::DiagnosticContext::getProfilerController() const
{
    return m_profilerController;
}

ego::context::DiagnosticContextPointer ego::context::GetDiagnosticContextPointer()
{
    return FindGlobalContext<DiagnosticContext>();
}

ego::context::DiagnosticContext& ego::context::GetDiagnosticContext()
{
    const DiagnosticContextPointer diagnosticContext = GetDiagnosticContextPointer();
    EGO_ASSERT(diagnosticContext);

    return *diagnosticContext;
}

ego::AssertGeneratorPointer ego::context::GetAssertGenerator()
{
    const DiagnosticContextPointer diagnosticContext = GetDiagnosticContextPointer();
    if (diagnosticContext && diagnosticContext->getAssertGenerator())
    {
        return diagnosticContext->getAssertGenerator();
    }

    return nullptr;
}
