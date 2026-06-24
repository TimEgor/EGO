#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::framework
{
    class GameLogic : public NonCopyable
    {
    public:
        GameLogic() = default;
        ~GameLogic() override = default;

        virtual bool init()
        {
            return true;
        }
        virtual void update(float) {}
        virtual void release() {}
    };

    EGO_POINTER(GameLogic);
    EGO_WEAK_POINTER(GameLogic);
} // namespace ego::framework
