#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "Application.h"

namespace ego::application
{
    class ApplicationSubsystem final : public subsystem::Subsystem
    {
    public:
        ApplicationSubsystem() = default;
        ~ApplicationSubsystem() override = default;

        bool init(const ApplicationPointer& _application);
        void release() override;

        ApplicationPointer getApplicationPointer() const;
        Application& getApplication() const;

        EGO_SUBSYSTEM(ApplicationSubsystem, subsystem::Subsystem);

    private:
        ApplicationWeakPointer m_application;
    };

    ApplicationSubsystemPointer GetApplicationSubsystemPointer();
    ApplicationSubsystem& GetApplicationSubsystem();

    ApplicationPointer GetApplicationPointer();
    Application& GetApplication();
} // namespace ego::application
