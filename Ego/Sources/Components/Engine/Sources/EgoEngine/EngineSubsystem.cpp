#include "EngineSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

bool ego::engine::EngineSubsystem::init()
{
    EGO_CHECK_INITIALIZATION(!m_engine);

    m_engine = MakePointer<Engine>();
    EGO_CHECK_INITIALIZATION(m_engine && m_engine->init());

    return true;
}

void ego::engine::EngineSubsystem::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

ego::engine::EnginePointer ego::engine::EngineSubsystem::getEnginePointer() const
{
    return m_engine;
}

ego::engine::Engine& ego::engine::EngineSubsystem::getEngine() const
{
    const EnginePointer engine = getEnginePointer();
    EGO_ASSERT(engine);

    return *engine;
}

ego::engine::EngineSubsystemPointer ego::engine::GetEngineSubsystemPointer()
{
    return subsystem::FindSubsystem<EngineSubsystem>();
}

ego::engine::EngineSubsystem& ego::engine::GetEngineSubsystem()
{
    const EngineSubsystemPointer engineSubsystem = GetEngineSubsystemPointer();
    EGO_ASSERT(engineSubsystem);

    return *engineSubsystem;
}
