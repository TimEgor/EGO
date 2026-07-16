#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Input/GuiInput.h"

#include "GuiViewportTypes.h"

namespace ego::gui
{
    using GuiViewportEventCollection = std::vector<GuiInputEvent>;

    struct GuiViewportUpdate final
    {
        GuiSize m_size = GuiSizeZero;
        GuiViewportEventCollection m_events;
        bool m_closeRequested = false;
    };

    class GuiViewportBackend
    {
    public:
        virtual ~GuiViewportBackend() = default;

        virtual bool createViewport(const GuiViewportCreateRequest& _request) = 0;
        virtual void destroyViewport(GuiViewportID _viewportID) = 0;
        virtual bool updateViewport(GuiViewportID _viewportID, GuiViewportUpdate& _update) = 0;
    };

    EGO_POINTER(GuiViewportBackend);
} // namespace ego::gui
