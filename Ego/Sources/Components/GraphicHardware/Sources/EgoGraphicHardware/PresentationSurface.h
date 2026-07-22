#pragma once

#include "EgoCore/Math/Vector.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego
{
    using PresentationSurfaceSize = UInt16Vector2;
    using PresentationSurfacePoint = Int32Vector2;
    inline constexpr PresentationSurfaceSize PresentationSurfaceSizeZero = UInt16Vector2Zero;
    inline constexpr PresentationSurfacePoint PresentationSurfacePointZero = Int32Vector2Zero;

    class PresentationSurface
    {
    public:
        PresentationSurface() = default;
        virtual ~PresentationSurface() = default;

        virtual void* getNativeHandle() const = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;
        virtual const PresentationSurfaceSize& getSize() const = 0;
        virtual bool screenToLocal(const PresentationSurfacePoint& _screenPoint, PresentationSurfacePoint& _localPoint) const = 0;

        EGO_RTTI_VIRTUAL_BASE(PresentationSurface);
    };

    EGO_POINTER(PresentationSurface);
} // namespace ego
