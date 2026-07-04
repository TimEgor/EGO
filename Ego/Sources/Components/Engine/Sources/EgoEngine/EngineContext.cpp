#include "EngineContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

bool ego::engine::EngineContext::init(const InitData& _initData)
{
    m_engine = new Engine();
    EGO_CHECK_INITIALIZATION(m_engine && m_engine->init(_initData.m_engineInitData));

    return true;
}

void ego::engine::EngineContext::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_engine);
}

ego::engine::EnginePointer ego::engine::EngineContext::getEnginePointer() const
{
    return m_engine;
}

ego::engine::Engine& ego::engine::EngineContext::getEngine() const
{
    EGO_ASSERT(m_engine);
    return *m_engine;
}
