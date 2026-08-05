#pragma once

#include "EgoCore/RTTI/Type/TypeMetaInfoMacros.h"

namespace ego::ecs
{
    struct Component
    {
        virtual ~Component() = default;

        EGO_RTTI_VIRTUAL_BASE(Component);
    };
} // namespace ego::ecs
