#pragma once

#include <string>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoEvent/EventController.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"
#include "EgoGraphicHardware/PresentationSurface.h"

namespace ego::application
{
    struct PresentationDesc final
    {
        std::string m_name;
        PresentationSurfaceSize m_size = PresentationSurfaceSizeZero;
        bool m_isVisible = false;
    };

    struct PresentationSurfaceEventIDs final
    {
        InstancedEventID m_destroying = InvalidInstancedEventID;
        InstancedEventID m_activation = InvalidInstancedEventID;
        InstancedEventID m_sizeChanged = InvalidInstancedEventID;
        InstancedEventID m_keyboardInput = InvalidInstancedEventID;
        InstancedEventID m_textInput = InvalidInstancedEventID;
    };

    struct Presentation final
    {
        PresentationSurfacePointer m_surface = nullptr;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        PresentationSurfaceEventIDs m_surfaceEventIDs;
    };

    class PresenterProvider
    {
    public:
        virtual ~PresenterProvider() = default;

        virtual void release() = 0;

        virtual Presentation createPresentation(const PresentationDesc& _desc) = 0;
        virtual bool destroyPresentation(const PresentationSurfacePointer& _surface) = 0;
        virtual GraphicPresenterPointer findGraphicPresenter(const PresentationSurfacePointer& _surface) const = 0;
        virtual void processEvents() = 0;

        EGO_RTTI_VIRTUAL_BASE(PresenterProvider);
    };

    EGO_POINTER(PresenterProvider);
} // namespace ego::application
