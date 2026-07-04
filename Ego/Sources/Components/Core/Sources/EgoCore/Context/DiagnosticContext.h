#pragma once

#include "EgoCore/Assert/AssertGenerator.h"
#include "EgoCore/Log/Log.h"
#include "EgoCore/Profile/Profile.h"
#include "GlobalContext.h"

namespace ego::context
{
    class DiagnosticContext final : public GlobalContext
    {
    public:
        DiagnosticContext() = default;
        ~DiagnosticContext() override = default;

        void release();

        void setAssertGenerator(const AssertGeneratorPointer& _assertGenerator);
        AssertGeneratorPointer getAssertGenerator() const;

        void setLogger(const log::LoggerPointer& _logger);
        log::LoggerPointer getLogger() const;

        void setProfilerController(const profile::ProfilerControllerPointer& _profilerController);
        profile::ProfilerControllerPointer getProfilerController() const;

        EGO_RTTI_VIRTUAL(DiagnosticContext, GlobalContext);

    private:
        AssertGeneratorPointer m_assertGenerator = nullptr;
        log::LoggerPointer m_logger = nullptr;
        profile::ProfilerControllerPointer m_profilerController = nullptr;
    };

    EGO_POINTER(DiagnosticContext);

    DiagnosticContextPointer GetDiagnosticContextPointer();
    DiagnosticContext& GetDiagnosticContext();

    AssertGeneratorPointer GetAssertGenerator();
} // namespace ego::context
