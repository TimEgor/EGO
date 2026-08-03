#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "Application.h"

namespace ego::application
{
    class ApplicationSubsystem final
        : public subsystem::Subsystem,
          public EnableSharedFromThis<ApplicationSubsystem>
    {
    public:
        ApplicationSubsystem() = default;
        ~ApplicationSubsystem() override;

        bool init(const Application::InitData& _initData);

        ApplicationPointer getApplicationPointer() const;
        Application& getApplication() const;

        EGO_SUBSYSTEM(ApplicationSubsystem, subsystem::Subsystem);

    private:
        void release();

        ApplicationPointer m_application = nullptr;
    };

    ApplicationSubsystemPointer GetApplicationSubsystemPointer();
    ApplicationSubsystem& GetApplicationSubsystem();

    ApplicationPointer GetApplicationPointer();
    Application& GetApplication();
} // namespace ego::application
