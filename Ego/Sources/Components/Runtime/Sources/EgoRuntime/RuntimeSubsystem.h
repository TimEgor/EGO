#pragma once

#include "EgoCore/Subsystem/Subsystem.h"

#include "Runtime.h"

namespace ego::runtime
{
    class RuntimeSubsystem final : public subsystem::Subsystem
    {
    public:
        RuntimeSubsystem() = default;
        ~RuntimeSubsystem() override;

        bool init(const Runtime::InitData& _initData);

        RuntimePointer getRuntimePointer() const;
        Runtime& getRuntime() const;

        EGO_SUBSYSTEM(RuntimeSubsystem, subsystem::Subsystem);

    private:
        void release();

        RuntimePointer m_runtime = nullptr;
    };

    EGO_POINTER(RuntimeSubsystem);

    RuntimeSubsystemPointer GetRuntimeSubsystemPointer();
    RuntimeSubsystem& GetRuntimeSubsystem();

    RuntimePointer GetRuntimePointer();
    Runtime& GetRuntime();
} // namespace ego::runtime
