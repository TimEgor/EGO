#pragma once

#include <cstddef>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Core/Geometry.h"
#include "EgoGui/Input/Input.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"

namespace ego::gui
{
    class WidgetUpdateContext;

    class Widget;

    EGO_POINTER(Widget);
    EGO_WEAK_POINTER(Widget);

    enum class Visibility
    {
        Visible,
        Hidden,
        Collapsed
    };

    class Widget : public NonCopyable, public ego::EnableSharedFromThis<Widget>
    {
        friend class WidgetUpdateContext;

    public:
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

        void invalidateLayout() const;
        bool isLayoutInvalidated() const;

        virtual size_t getChildCount() const;
        virtual WidgetPointer getChild(size_t _index) const;
        virtual bool isChildActive(size_t _index) const;
        virtual bool hitTest(const Position& _position) const;
        virtual bool isChildHitTestVisible(const Position& _position) const;

        WidgetPointer getParent() const;
        bool isDescendantOf(const Widget& _ancestor) const;
        bool isDirectChildOf(const Widget& _parent) const;

        EGO_RTTI_VIRTUAL_BASE(Widget);

    protected:
        bool attachChild(const WidgetPointer& _child);
        bool detachChild(const WidgetPointer& _child);
        void notifyTreeChanged() const;
        void completeLayout();

        virtual InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event);
        virtual InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event);
        virtual InputReply onMouseWheel(WidgetUpdateContext& _context, const MouseWheelEvent& _event);
        virtual InputReply onKey(WidgetUpdateContext& _context, const KeyEvent& _event);
        virtual InputReply onTextInput(WidgetUpdateContext& _context, const TextInputEvent& _event);

        virtual void onPointerEnter(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerLeave(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers);
        virtual void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position);
        virtual void onFocusChanged(WidgetUpdateContext& _context, FocusChange _change);

        virtual Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints);
        virtual void updateGeometry(const LayoutContext& _context);
        virtual void onLayoutCompleted();
        virtual void drawBaseLayer(PaintContext& _context) const;
        virtual void drawOverlayLayer(PaintContext& _context) const;
        virtual bool clipsChildren() const;
        virtual Rect getChildrenClipRect() const;

    private:
        WidgetWeakPointer m_parent;
        Visibility m_visibility = Visibility::Visible;
        Size m_preferredSize = SizeZero;
        Rect m_layoutBounds;
        mutable bool m_isLayoutInvalidated = true;
    };
} // namespace ego::gui
