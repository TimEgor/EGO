#pragma once

#include "EgoCore/Platform/Surface/PlatformSurface.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

namespace ego::application
{
    using PresentationDesc = PlatformSurfaceDesc;

    struct Presentation final
    {
        PlatformSurfacePointer m_surface = nullptr;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
    };

    class PresenterProvider
    {
    public:
        virtual ~PresenterProvider() = default;

        virtual void release() = 0;

        virtual Presentation createPresentation(const PresentationDesc& _desc) = 0;
        virtual bool destroyPresentation(const PlatformSurfacePointer& _surface) = 0;
        virtual GraphicPresenterPointer findGraphicPresenter(const PlatformSurfacePointer& _surface) const = 0;
        virtual void processEvents() = 0;

        EGO_RTTI_VIRTUAL_BASE(PresenterProvider);
    };

    EGO_POINTER(PresenterProvider);
} // namespace ego::application
