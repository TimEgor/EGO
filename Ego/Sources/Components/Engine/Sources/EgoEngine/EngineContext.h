#pragma once

#include "EgoCore/Context/Context.h"

#include "Engine.h"

namespace ego::engine
{
    class EngineContext final : public context::Context
    {
    public:
        struct InitData final
        {
            Engine::InitData m_engineInitData;
        };

        EngineContext() = default;
        ~EngineContext() override = default;

        bool init(const InitData& _initData);
        void release();

        EnginePointer getEnginePointer() const;
        Engine& getEngine() const;

        EGO_RTTI_VIRTUAL(EngineContext, context::Context);

    private:
        EnginePointer m_engine = nullptr;
    };

    EGO_POINTER(EngineContext);
} // namespace ego::engine
