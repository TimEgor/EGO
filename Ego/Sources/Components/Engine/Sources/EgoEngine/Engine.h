#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EngineSession.h"

namespace ego
{
    class JobController;

    EGO_POINTER(JobController);
} // namespace ego

namespace ego::engine
{
    class Engine final : public NonCopyable
    {
    public:
        using SessionCollection = std::vector<EngineSessionPointer>;

        Engine() = default;
        ~Engine() override;

        bool init();

        EngineSessionPointer createSession(const EngineSession::InitData& _initData);
        bool destroySession(EngineSessionID _sessionID);

        EngineSessionPointer findSession(EngineSessionID _sessionID) const;
        const SessionCollection& getSessions() const;

        bool tick();

    private:
        void release();

        bool initJobController();
        void releaseJobController();

        EngineSessionID allocateSessionID();

        JobControllerPointer m_jobController = nullptr;
        SessionCollection m_sessions;
        EngineSessionID m_nextSessionID = 1;
    };

    EGO_POINTER(Engine);
} // namespace ego::engine
