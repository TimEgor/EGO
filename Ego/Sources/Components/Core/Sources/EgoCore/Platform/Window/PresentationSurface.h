#pragma once

#include "EgoCore/RTTI/RTTI.h"
#include "WindowTypes.h"

namespace ego
{
    using PresentationSurfaceSize = WindowSize;

    class PresentationSurface
    {
    public:
        PresentationSurface() = default;
        virtual ~PresentationSurface() = default;

        virtual void* getNativeHandle() const = 0;
        virtual const PresentationSurfaceSize& getClientAreaSize() const = 0;

        EGO_RTTI_VIRTUAL_BASE(PresentationSurface);
    };
} // namespace ego
