#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego::context
{
    class Context
    {
    public:
        Context() = default;
        virtual ~Context() = default;

        EGO_RTTI_VIRTUAL_BASE(Context);
    };

    EGO_POINTER(Context);
} // namespace ego::context
