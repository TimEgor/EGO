#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Core/Geometry.h"
#include "EgoGui/Input/Input.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"

namespace ego::gui
{
    class SurfaceRoot;
    class Widget;

    EGO_POINTER(Widget);
    EGO_WEAK_POINTER(Widget);

    enum class Visibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    class Widget
        : public NonCopyable,
          public ego::EnableSharedFromThis<Widget>
    {
    public:
        class WidgetAccessor final
            : public NonInstanceable
        {
        public:
            static InputReply HandleInput(Widget& _widget, const PointerMoveEvent& _event);
            static InputReply HandleInput(Widget& _widget, const MouseButtonEvent& _event);
            static InputReply HandleInput(Widget& _widget, const MouseWheelEvent& _event);
            static InputReply HandleInput(Widget& _widget, const KeyEvent& _event);
            static InputReply HandleInput(Widget& _widget, const TextInputEvent& _event);

            static void NotifyPointerEnter(Widget& _widget, const Position& _position, const InputModifiers& _modifiers);
            static void NotifyPointerLeave(Widget& _widget, const Position& _position, const InputModifiers& _modifiers);
            static void NotifyPointerCaptureLost(Widget& _widget, const Position& _position);
            static void NotifyFocusChanged(Widget& _widget, FocusChange _change);
        };

        Widget();
        virtual ~Widget() = default;

        void setVisibility(Visibility _visibility);
        Visibility getVisibility() const;
        bool isVisible() const;
        bool isCollapsed() const;

        const Size& getPreferredSize() const;
        const Rect& getLayoutBounds() const;

        Size updatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints);
        void applyLayout(const LayoutContext& _context, const Rect& _bounds);
        void emitDrawCommands(PaintContext& _context) const;

        virtual size_t getChildCount() const;
        virtual const WidgetPointer& getChild(size_t _index) const;
        virtual bool isChildHitTestVisible(const Position& _position) const;
        virtual Rect resolveTopLevelBounds(const Rect& _surfaceBounds) const;

        WidgetPointer getParent() const;
        bool isAttachedTo(const SurfaceRoot& _surfaceRoot) const;
        bool isDirectChildOf(const Widget& _parent) const;

        EGO_RTTI_VIRTUAL_BASE(Widget);

    protected:
        bool attachChild(const WidgetPointer& _child);
        bool detachChild(const WidgetPointer& _child);
        bool canMutateTree() const;
        void invalidateLayout() const;
        void notifyTreeChanged() const;

        virtual InputReply onPointerMove(const PointerMoveEvent& _event);
        virtual InputReply onMouseButton(const MouseButtonEvent& _event);
        virtual InputReply onMouseWheel(const MouseWheelEvent& _event);
        virtual InputReply onKey(const KeyEvent& _event);
        virtual InputReply onTextInput(const TextInputEvent& _event);

        virtual void onPointerEnter(const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerLeave(const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerCaptureLost(const Position& _position);
        virtual void onFocusChanged(FocusChange _change);

        virtual Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints);
        virtual void updateGeometry(const LayoutContext& _context);
        virtual void drawBaseLayer(PaintContext& _context) const;
        virtual void drawOverlayLayer(PaintContext& _context) const;
        virtual bool clipsChildren() const;
        virtual Rect getChildrenClipRect() const;

        void bindSurfaceRoot(const ego::WeakPointer<SurfaceRoot>& _surfaceRoot);

    private:
        WidgetWeakPointer m_parent;
        ego::WeakPointer<SurfaceRoot> m_surfaceRoot;
        Visibility m_visibility = Visibility::Visible;
        Size m_preferredSize = SizeZero;
        Rect m_layoutBounds;
    };
} // namespace ego::gui
