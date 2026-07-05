#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "GuiInput.h"

namespace ego::gui
{
    class GuiViewport;

    class GuiViewportInputAdapter
    {
    public:
        GuiViewportInputAdapter() = default;
        virtual ~GuiViewportInputAdapter() = default;

        virtual bool adaptInputEvent(GuiInputEvent& _event, const GuiViewport& _viewport) const = 0;

        EGO_RTTI_VIRTUAL_BASE(GuiViewportInputAdapter);
    };

    EGO_POINTER(GuiViewportInputAdapter);
} // namespace ego::gui
