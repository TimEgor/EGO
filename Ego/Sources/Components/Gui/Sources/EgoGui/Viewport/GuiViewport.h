#pragma once

#include <vector>

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Input/GuiInput.h"
#include "EgoGui/Widgets/GuiWindow.h"

#include "GuiViewportTypes.h"

namespace ego::gui
{
    class GuiViewport final
    {
    public:
        // TODO: Profile window traversal and Z-order changes before considering a different container.
        using WindowCollection = std::vector<GuiWindowPointer>;

        GuiViewport(GuiViewportID _id, GuiViewportRole _role, const GuiSize& _size);

        GuiViewportID getID() const;
        GuiViewportRole getRole() const;
        const GuiSize& getSize() const;
        void setSize(const GuiSize& _size);

        bool addWindow(const GuiWindowPointer& _window);
        GuiWindowPointer removeWindow(const GuiWindowPointer& _window);
        void clearWindows();
        const WindowCollection& getWindows() const;
        GuiWindowPointer getActiveWindow() const;

        GuiEventResult processEvent(const GuiInputEvent& _event);
        void buildDrawData(const GuiLayoutContext& _layoutContext, GuiPaintContext& _paintContext);

        EGO_RTTI_VIRTUAL_BASE(GuiViewport);

    private:
        GuiEventResult processFocusLostEvent(const GuiInputEvent& _event);
        GuiEventResult processPointerEvent(const GuiInputEvent& _event);
        GuiEventResult dispatchToActiveWindow(const GuiInputEvent& _event);

        void updateWindowLayouts(const GuiLayoutContext& _context);
        void paintWindows(GuiPaintContext& _context) const;
        bool containsPosition(const GuiPosition& _position) const;
        GuiWindowPointer findWindowAtPosition(const GuiPosition& _position) const;
        void setActiveWindow(const GuiWindowPointer& _window);
        void updateHoveredWindow(const GuiWindowPointer& _window, const GuiPosition& _position);
        void bringWindowToFront(const GuiWindowPointer& _window);

        GuiViewportID m_id = InvalidGuiViewportID;
        GuiViewportRole m_role = GuiViewportRole::Secondary;
        GuiSize m_size = GuiSizeZero;
        WindowCollection m_windows;
        GuiWindowWeakPointer m_activeWindow;
        GuiWindowWeakPointer m_hoveredWindow;
        GuiWindowWeakPointer m_capturedWindow;
        GuiMouseButton m_capturedMouseButton = GuiMouseButton::Left;
    };

    EGO_POINTER(GuiViewport);
} // namespace ego::gui
