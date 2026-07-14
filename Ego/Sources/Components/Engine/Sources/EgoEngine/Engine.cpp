#include "Engine.h"

#include <algorithm>

#include "EgoCore/UtilsMacros.h"

#include "EgoJob/JobController.h"

ego::engine::Engine::~Engine()
{
    release();
}

bool ego::engine::Engine::init()
{
    EGO_CHECK_RETURN_FALSE(!m_isInitialized);
    EGO_CHECK_RETURN_FALSE(!m_jobController);
    EGO_CHECK_RETURN_FALSE(m_sessions.empty());

    EGO_CHECK_INITIALIZATION(initJobController());

    m_isInitialized = true;
    return true;
}

void ego::engine::Engine::release()
{
    while (!m_sessions.empty())
    {
        const EngineSessionPointer session = m_sessions.back();
        m_sessions.pop_back();
        if (session)
        {
            session->release();
        }
    }

    releaseJobController();

    m_nextSessionID = 1;
    m_isInitialized = false;
}

bool ego::engine::Engine::isInitialized() const
{
    return m_isInitialized;
}

ego::engine::EngineSessionPointer ego::engine::Engine::createSession(const EngineSession::InitData& _initData)
{
    EGO_CHECK_RETURN_NULL(m_isInitialized);
    EGO_CHECK_RETURN_NULL(m_jobController);

    const EngineSessionID sessionID = allocateSessionID();
    EGO_CHECK_RETURN_NULL(sessionID != InvalidEngineSessionID);

    EngineSessionPointer session = new EngineSession();
    EGO_CHECK_RETURN_NULL(session);
    EGO_CHECK_RETURN_NULL(session->init(m_jobController, sessionID, _initData));

    m_sessions.push_back(session);
    return session;
}

bool ego::engine::Engine::destroySession(EngineSessionID _sessionID)
{
    const SessionCollection::iterator sessionIt = std::find_if(
        m_sessions.begin(),
        m_sessions.end(),
        [_sessionID](const EngineSessionPointer& _session)
        {
            return _session && _session->getID() == _sessionID;
        });
    if (sessionIt == m_sessions.end())
    {
        return false;
    }

    const EngineSessionPointer session = *sessionIt;
    m_sessions.erase(sessionIt);
    session->release();
    return true;
}

ego::engine::EngineSessionPointer ego::engine::Engine::findSession(EngineSessionID _sessionID) const
{
    for (const EngineSessionPointer& session : m_sessions)
    {
        if (session && session->getID() == _sessionID)
        {
            return session;
        }
    }

    return nullptr;
}

const ego::engine::Engine::SessionCollection& ego::engine::Engine::getSessions() const
{
    return m_sessions;
}

bool ego::engine::Engine::tick()
{
    EGO_CHECK_RETURN_FALSE(m_isInitialized);

    for (const EngineSessionPointer& session : m_sessions)
    {
        EGO_CHECK_RETURN_FALSE(session && session->tick());
    }

    return true;
}

bool ego::engine::Engine::initJobController()
{
    EGO_CHECK_RETURN_FALSE(!m_jobController);

    uint32_t threadCount = JobController::GetHardwareThreadCount();
    if (threadCount == 0)
    {
        threadCount = 1;
    }

    m_jobController = new JobController();
    return m_jobController && m_jobController->init(threadCount, "EgoJob");
}

void ego::engine::Engine::releaseJobController()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_jobController);
}

ego::engine::EngineSessionID ego::engine::Engine::allocateSessionID()
{
    const EngineSessionID firstCandidate = m_nextSessionID;

    do
    {
        const EngineSessionID candidate = m_nextSessionID++;
        if (m_nextSessionID == InvalidEngineSessionID)
        {
            m_nextSessionID = 1;
        }

        if (candidate != InvalidEngineSessionID && !findSession(candidate))
        {
            return candidate;
        }
    } while (m_nextSessionID != firstCandidate);

    return InvalidEngineSessionID;
}
