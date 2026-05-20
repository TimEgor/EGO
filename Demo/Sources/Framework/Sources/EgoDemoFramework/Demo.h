#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego::demo
{
    class Demo : public NonCopyable
    {
    public:
        Demo() = default;
        virtual ~Demo() = default;

        virtual bool init() { return true; }
        virtual void release() {}
    };

    EGO_POINTER(Demo);
    EGO_WEAK_POINTER(Demo);
}
