#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::engine_framework
{
    class EngineLogic : public NonCopyable
    {
    public:
        EngineLogic() = default;
        ~EngineLogic() override = default;

        virtual bool init()
        {
            return true;
        }
        virtual void update(float) {}
        virtual void release() {}
    };

    EGO_POINTER(EngineLogic);
    EGO_WEAK_POINTER(EngineLogic);
} // namespace ego::engine_framework
