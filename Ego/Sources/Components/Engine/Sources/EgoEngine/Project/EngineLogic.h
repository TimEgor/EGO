#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

namespace ego::engine
{
    class EngineSession;

    EGO_POINTER(EngineSession);
    EGO_WEAK_POINTER(EngineSession);

    class EngineLogic : public NonCopyable
    {
    public:
        EngineLogic() = default;
        ~EngineLogic() override = default;

        virtual bool init(const EngineSessionWeakPointer& _engineSession)
        {
            return true;
        }

        virtual void update(float) {}
    };

    EGO_POINTER(EngineLogic);
    EGO_WEAK_POINTER(EngineLogic);
} // namespace ego::engine
