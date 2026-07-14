#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

namespace ego
{
    class PluginController;

    EGO_POINTER(PluginController);

    class PlatformPluginSubsystem final : public subsystem::Subsystem
    {
    public:
        PlatformPluginSubsystem() = default;
        ~PlatformPluginSubsystem() override = default;

        bool init();
        void release() override;

        PluginControllerPointer getPluginControllerPointer() const;
        PluginController& getPluginController() const;

        EGO_SUBSYSTEM(PlatformPluginSubsystem, subsystem::Subsystem);

    private:
        PluginControllerPointer m_pluginController = nullptr;
    };

    EGO_POINTER(PlatformPluginSubsystem);

    PlatformPluginSubsystemPointer GetPlatformPluginSubsystemPointer();
    PlatformPluginSubsystem& GetPlatformPluginSubsystem();
} // namespace ego
