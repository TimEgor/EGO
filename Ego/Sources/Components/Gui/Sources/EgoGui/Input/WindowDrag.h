#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class InputState;
    class SurfaceRoot;
    class ViewportInputContext;
    class Widget;
    class Window;

    EGO_POINTER(Widget);
    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    struct WindowDragContext final
    {
        InputState& m_inputState;
        SurfaceRoot& m_root;
        ViewportInputContext& m_viewportInput;
    };

    class WindowDrag final : public NonCopyable
    {
    public:
        void beginFloating(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition);
        void beginDocked(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition);
        bool update(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition);
        void finish(WindowDragContext& _context, const WindowPointer& _window, const Position& _position, const Position& _screenPosition);
        bool cancel(WindowDragContext& _context, const WindowPointer& _window);
        bool clearLocal(SurfaceRoot& _root, const WindowPointer& _window);

        bool isActive(const WindowPointer& _window) const;
        bool hasPointerCapture() const;
        WindowPointer getWindow() const;

    private:
        enum class Origin
        {
            None,
            FloatingWindow,
            DockingTab
        };

        struct State final
        {
            Origin m_origin = Origin::None;
            WindowWeakPointer m_window;
            Position m_startPosition = PositionZero;
            Position m_startScreenPosition = PositionZero;
            Rect m_startBounds;
            Rect m_floatingBounds;
            bool m_hasPassedThreshold = false;
            bool m_hasPointerCapture = false;
            bool m_isViewportDragActive = false;
        };

        bool takePointerCapture(WindowDragContext& _context, const WidgetPointer& _source);
        bool makeDockedWindowFloating(WindowDragContext& _context, const WindowPointer& _window);
        void begin(
            SurfaceRoot& _root,
            const WindowPointer& _window,
            Origin _origin,
            const Position& _position,
            const Position& _screenPosition,
            const Rect& _floatingBounds);
        Rect constrainFloatingBounds(const SurfaceRoot& _root, const Rect& _bounds) const;
        void clear(SurfaceRoot& _root);
        void clearState();

        State m_state;
    };
} // namespace ego::gui
