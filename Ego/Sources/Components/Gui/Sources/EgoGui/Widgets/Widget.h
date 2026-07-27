#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Core/Geometry.h"

namespace ego::gui
{
    class InputContext;
    class InputPass;
    class InputState;
    class PaintContext;

    class Widget;

    EGO_POINTER(Widget);
    EGO_WEAK_POINTER(Widget);

    struct InputModifiers;
    struct KeyEvent;
    struct LayoutConstraints;
    struct LayoutContext;
    struct MouseButtonEvent;
    struct MouseWheelEvent;
    struct PointerMoveEvent;
    struct TextInputEvent;

    enum class FocusChange;
    enum class InputReply;

    enum class Visibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    class Widget : public NonCopyable, public ego::EnableSharedFromThis<Widget>
    {
    public:
        class WidgetAccessor final : public NonInstanceable
        {
            friend class InputPass;
            friend class InputState;
            friend class PaintContext;
            friend struct LayoutContext;

            static InputReply OnPointerMove(Widget& _widget, InputContext& _context, const PointerMoveEvent& _event);
            static InputReply OnMouseButton(Widget& _widget, InputContext& _context, const MouseButtonEvent& _event);
            static InputReply OnMouseWheel(Widget& _widget, InputContext& _context, const MouseWheelEvent& _event);
            static InputReply OnKey(Widget& _widget, InputContext& _context, const KeyEvent& _event);
            static InputReply OnTextInput(Widget& _widget, InputContext& _context, const TextInputEvent& _event);
            static void OnPointerEnter(Widget& _widget, const Position& _position, const InputModifiers& _modifiers);
            static void OnPointerLeave(Widget& _widget, const Position& _position, const InputModifiers& _modifiers);
            static void OnPointerCaptureLost(Widget& _widget, const Position& _position);
            static void OnFocusChanged(Widget& _widget, FocusChange _change);

            static Size UpdatePreferredSize(Widget& _widget, const LayoutContext& _context, const LayoutConstraints& _constraints);
            static void ApplyLayout(Widget& _widget, const LayoutContext& _context, const Rect& _bounds);
            static bool IsLayoutInvalidated(const Widget& _widget);
            static void OnLayoutCompleted(Widget& _widget);

            static void DrawBaseLayer(const Widget& _widget, PaintContext& _context);
            static void DrawOverlayLayer(const Widget& _widget, PaintContext& _context);
            static bool ClipsChildren(const Widget& _widget);
            static Rect GetChildrenClipRect(const Widget& _widget);

            static size_t GetChildCount(const Widget& _widget);
            static WidgetPointer GetChild(const Widget& _widget, size_t _index);
            static bool IsChildActive(const Widget& _widget, size_t _index);
            static bool HitTest(const Widget& _widget, const Position& _position);
            static bool IsChildHitTestVisible(const Widget& _widget, const Position& _position);
        };

        Widget();
        virtual ~Widget() = default;

        void setVisibility(Visibility _visibility);
        Visibility getVisibility() const;
        bool isVisible() const;
        bool isCollapsed() const;

        const Size& getPreferredSize() const;
        const Rect& getLayoutBounds() const;

        WidgetPointer getParent() const;
        bool isDescendantOf(const Widget& _ancestor) const;
        bool isDirectChildOf(const Widget& _parent) const;

        EGO_RTTI_VIRTUAL_BASE(Widget);

    protected:
        bool attachChild(const WidgetPointer& _child);
        bool detachChild(const WidgetPointer& _child);
        void invalidateLayout() const;
        void notifyTreeChanged() const;

        virtual InputReply onPointerMove(InputContext& _context, const PointerMoveEvent& _event);
        virtual InputReply onMouseButton(InputContext& _context, const MouseButtonEvent& _event);
        virtual InputReply onMouseWheel(InputContext& _context, const MouseWheelEvent& _event);
        virtual InputReply onKey(InputContext& _context, const KeyEvent& _event);
        virtual InputReply onTextInput(InputContext& _context, const TextInputEvent& _event);

        virtual void onPointerEnter(const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerLeave(const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerCaptureLost(const Position& _position);
        virtual void onFocusChanged(FocusChange _change);

        virtual Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints);
        virtual void updateGeometry(const LayoutContext& _context);
        virtual void onLayoutCompleted();
        virtual void drawBaseLayer(PaintContext& _context) const;
        virtual void drawOverlayLayer(PaintContext& _context) const;
        virtual bool clipsChildren() const;
        virtual Rect getChildrenClipRect() const;

        virtual size_t getChildCount() const;
        virtual WidgetPointer getChild(size_t _index) const;
        virtual bool isChildActive(size_t _index) const;
        virtual bool hitTest(const Position& _position) const;
        virtual bool isChildHitTestVisible(const Position& _position) const;

    private:
        Size updatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints);
        void applyLayout(const LayoutContext& _context, const Rect& _bounds);
        bool isLayoutInvalidated() const;

        WidgetWeakPointer m_parent;
        Visibility m_visibility = Visibility::Visible;
        Size m_preferredSize = SizeZero;
        Rect m_layoutBounds;
        mutable bool m_isLayoutInvalidated = true;
    };
} // namespace ego::gui
