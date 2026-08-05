#pragma once

#include "EgoCore/Subsystem/Subsystem.h"
#include "Platform.h"

namespace ego
{
    class PlatformSubsystem final : public subsystem::Subsystem
    {
    public:
        struct InitData final
        {
            void* m_nativeInstanceHandle = nullptr;
        };

        PlatformSubsystem() = default;
        ~PlatformSubsystem() override;

        bool init(const InitData& _initData);

        PlatformPointer getPlatformPointer() const;
        Platform& getPlatform() const;

        EGO_SUBSYSTEM(PlatformSubsystem, subsystem::Subsystem);

    private:
        void onUnregistered() override;
        void release();

        PlatformPointer m_platform = nullptr;
    };

    EGO_POINTER(PlatformSubsystem);

    PlatformSubsystemPointer GetPlatformSubsystemPointer();
    PlatformSubsystem& GetPlatformSubsystem();

    PlatformPointer GetPlatformPointer();
    Platform& GetPlatform();
} // namespace ego
