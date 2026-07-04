#pragma once

#include "Context.h"

namespace ego::context
{
    class GlobalContext : public Context
    {
    public:
        GlobalContext() = default;
        ~GlobalContext() override = default;

        EGO_RTTI_VIRTUAL(GlobalContext, Context);
    };

    EGO_POINTER(GlobalContext);
} // namespace ego::context
