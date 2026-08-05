#include "RuntimeSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

ego::runtime::RuntimeSubsystem::~RuntimeSubsystem()
{
    release();
}

bool ego::runtime::RuntimeSubsystem::init(const Runtime::InitData& _initData)
{
    EGO_CHECK_INITIALIZATION_ASSERT(!m_runtime);

    m_runtime = MakePointer<Runtime>();
    EGO_CHECK_INITIALIZATION_ASSERT(m_runtime && m_runtime->init(_initData));

    return true;
}

void ego::runtime::RuntimeSubsystem::release()
{
    m_runtime = nullptr;
}

ego::runtime::RuntimePointer ego::runtime::RuntimeSubsystem::getRuntimePointer() const
{
    return m_runtime;
}

ego::runtime::Runtime& ego::runtime::RuntimeSubsystem::getRuntime() const
{
    const RuntimePointer runtime = getRuntimePointer();
    EGO_ASSERT(runtime);

    return *runtime;
}

ego::runtime::RuntimeSubsystemPointer ego::runtime::GetRuntimeSubsystemPointer()
{
    return subsystem::FindSubsystem<RuntimeSubsystem>();
}

ego::runtime::RuntimeSubsystem& ego::runtime::GetRuntimeSubsystem()
{
    const RuntimeSubsystemPointer runtimeSubsystem = GetRuntimeSubsystemPointer();
    EGO_ASSERT(runtimeSubsystem);

    return *runtimeSubsystem;
}

ego::runtime::RuntimePointer ego::runtime::GetRuntimePointer()
{
    const RuntimeSubsystemPointer runtimeSubsystem = GetRuntimeSubsystemPointer();

    return runtimeSubsystem ? runtimeSubsystem->getRuntimePointer() : nullptr;
}

ego::runtime::Runtime& ego::runtime::GetRuntime()
{
    const RuntimePointer runtime = GetRuntimePointer();
    EGO_ASSERT(runtime);

    return *runtime;
}
