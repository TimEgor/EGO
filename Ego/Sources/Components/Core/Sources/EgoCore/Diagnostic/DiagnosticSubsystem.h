#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

namespace ego::profile
{
    class ProfilerController;

    EGO_POINTER(ProfilerController);
} // namespace ego::profile

namespace ego::log
{
    class LoggerController;

    EGO_POINTER(LoggerController);
} // namespace ego::log

namespace ego
{
    class AssertController;

    EGO_POINTER(AssertController);

    class DiagnosticSubsystem final : public subsystem::Subsystem
    {
    public:
        DiagnosticSubsystem() = default;
        ~DiagnosticSubsystem() override;

        bool init();

        AssertControllerPointer getAssertController() const;
        log::LoggerControllerPointer getLoggerController() const;
        profile::ProfilerControllerPointer getProfilerController() const;

        EGO_SUBSYSTEM(DiagnosticSubsystem, subsystem::Subsystem);

    private:
        void release();

        AssertControllerPointer m_assertController = nullptr;
        log::LoggerControllerPointer m_loggerController = nullptr;
        profile::ProfilerControllerPointer m_profilerController = nullptr;
    };

    EGO_POINTER(DiagnosticSubsystem);

    DiagnosticSubsystemPointer GetDiagnosticSubsystemPointer();
    DiagnosticSubsystem& GetDiagnosticSubsystem();

    AssertControllerPointer GetAssertController();
    log::LoggerControllerPointer GetLoggerController();
    profile::ProfilerControllerPointer GetProfilerController();
} // namespace ego
