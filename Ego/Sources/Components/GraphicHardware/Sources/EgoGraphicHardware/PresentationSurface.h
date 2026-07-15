#pragma once

#include "EgoCore/Math/Vector.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    using PresentationSurfaceSize = UInt16Vector2;

    class PresentationSurface
    {
    public:
        PresentationSurface() = default;
        virtual ~PresentationSurface() = default;

        virtual void* getNativeHandle() const = 0;
        virtual const PresentationSurfaceSize& getSize() const = 0;

        EGO_RTTI_VIRTUAL_BASE(PresentationSurface);
    };
} // namespace ego
