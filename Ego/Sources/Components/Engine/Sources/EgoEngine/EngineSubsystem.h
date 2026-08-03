#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "Engine.h"

namespace ego::engine
{
    class EngineSubsystem final : public subsystem::Subsystem
    {
    public:
        EngineSubsystem() = default;
        ~EngineSubsystem() override;

        bool init();

        EnginePointer getEnginePointer() const;
        Engine& getEngine() const;

        EGO_SUBSYSTEM(EngineSubsystem, subsystem::Subsystem);

    private:
        void release();

        EnginePointer m_engine = nullptr;
    };

    EGO_POINTER(EngineSubsystem);

    EngineSubsystemPointer GetEngineSubsystemPointer();
    EngineSubsystem& GetEngineSubsystem();
} // namespace ego::engine
