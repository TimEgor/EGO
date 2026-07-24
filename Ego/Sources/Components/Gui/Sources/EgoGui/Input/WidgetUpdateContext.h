#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Input/Input.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class DockingArea;
    class DockingOverlay;
    class SurfaceRoot;

    EGO_POINTER(DockingArea);
    EGO_POINTER(DockingOverlay);

    class WidgetUpdateState final
    {
        friend class WidgetUpdateContext;

    public:
        WidgetUpdateState() = default;

    private:
        using MouseButtonMask = uint8_t;
        using WeakWidgetPath = std::vector<WidgetWeakPointer>;

        struct PointerState final
        {
            Position m_position = PositionZero;
            InputModifiers m_modifiers;
            WeakWidgetPath m_hoverPath;
            WidgetWeakPointer m_captureWidget;
            MouseButtonMask m_captureButtons = 0;
        };

        WidgetWeakPointer m_focusedWidget;
        PointerState m_pointer;
        WidgetWeakPointer m_pointerCaptureCancellationTarget;
        size_t m_dispatchDepth = 0;
        bool m_isClearPending = false;
    };

    class WidgetUpdateContext final
        : public NonCopyable
    {
    public:
        WidgetUpdateContext(SurfaceRoot& _root, WidgetUpdateState& _state);

        void update(const InputEvent& _event);
        void refresh();
        void clear();
        void detachInputState(const WidgetPointer& _subtree = nullptr);
        void requestPointerCaptureCancellation(const WidgetPointer& _relatedWidget = nullptr);
        void flushPointerCaptureCancellation();
        bool transferPointerCapture(const WidgetPointer& _source, const WidgetPointer& _target);

        WidgetPointer getFocusedWidget() const;
        SurfaceRoot& getRoot() const;
        DockingAreaPointer getDockingArea() const;
        DockingOverlayPointer getDockingOverlay() const;

    private:
        friend struct PointerMoveEvent;
        friend struct PointerExitEvent;
        friend struct MouseButtonEvent;
        friend struct MouseWheelEvent;
        friend struct KeyEvent;
        friend struct TextInputEvent;
        friend struct ViewportDeactivatedEvent;

        using MouseButtonMask = WidgetUpdateState::MouseButtonMask;
        using WidgetPath = std::vector<WidgetPointer>;

        static MouseButtonMask GetMouseButtonMask(MouseInputKey _key);
        static bool ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget);

        void updatePointerMove(const PointerMoveEvent& _event);
        void updatePointerExit(const PointerExitEvent& _event);
        void updateMouseButton(const MouseButtonEvent& _event);
        void updateMouseWheel(const MouseWheelEvent& _event);
        void updateKey(const KeyEvent& _event);
        void updateTextInput(const TextInputEvent& _event);
        void updateViewportDeactivated(const ViewportDeactivatedEvent& _event);

        WidgetPath buildPath(const WidgetPointer& _target) const;
        bool isPathResponderValid(const WidgetPath& _path, size_t _responderIndex) const;
        bool applyReply(const WidgetPointer& _responder, InputReply _reply);

        void setFocusedWidget(const WidgetPointer& _widget);
        bool capturePointer(const WidgetPointer& _widget, MouseInputKey _key);
        void releasePointerButton(MouseInputKey _key);
        void clearPointerCapture();
        void applyPointerCaptureCancellation();
        void clearState();
        void refreshState();
        void updateHover(const WidgetPath& _path);

        SurfaceRoot& m_root;
        DockingAreaPointer m_dockingArea;
        DockingOverlayPointer m_dockingOverlay;
        WidgetUpdateState& m_state;
    };
} // namespace ego::gui
