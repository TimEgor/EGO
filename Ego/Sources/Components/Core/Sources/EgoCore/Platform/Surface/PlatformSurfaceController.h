#pragma once

#include "PlatformSurface.h"

namespace ego
{
    class PlatformSurfaceController
    {
    public:
        PlatformSurfaceController() = default;
        virtual ~PlatformSurfaceController() = default;

        virtual PlatformSurfacePointer createSurface(const PlatformSurfaceDesc& _desc) = 0;
        virtual bool destroySurface(const PlatformSurfacePointer& _surface) = 0;
        virtual PlatformSurfacePointer findSurfaceAtPoint(const SurfacePoint& _point) const = 0;
        virtual bool setPointerCapture(const PlatformSurfacePointer& _surface) = 0;
        virtual PlatformSurfacePointer getPointerCapture() const = 0;
        virtual void processEvents() = 0;

        EGO_RTTI_VIRTUAL_BASE(PlatformSurfaceController);
    };

    EGO_POINTER(PlatformSurfaceController);
} // namespace ego
