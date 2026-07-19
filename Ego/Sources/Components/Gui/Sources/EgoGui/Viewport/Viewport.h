#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Input/Input.h"

#include "SurfaceRoot.h"
#include "ViewportTypes.h"

namespace ego::gui
{
    class GuiController;
    class InputRouter;
    struct ViewportUpdate;

    class Viewport final : public NonCopyable
    {
    public:
        class ViewportAccessor final : public NonInstanceable
        {
            friend class GuiController;

            static ego::SharedPointer<Viewport> Create(ViewportID _id, ViewportRole _role, const Size& _size);
            static void Update(Viewport& _viewport, const LayoutContext& _layoutContext, const ViewportUpdate& _update);
            static void ClearInteraction(Viewport& _viewport);
            static void InvalidateLayout(Viewport& _viewport);
            static void EmitDrawCommands(Viewport& _viewport, const LayoutContext& _layoutContext, PaintContext& _paintContext);
        };

        using WidgetCollection = SurfaceRoot::WidgetCollection;

        ~Viewport() override;

        ViewportID getID() const;
        ViewportRole getRole() const;
        const Size& getSize() const;
        void setSize(const Size& _size);

        bool add(const WidgetPointer& _widget);
        WidgetPointer remove(const WidgetPointer& _widget);
        void clear();
        const WidgetCollection& getWidgets() const;
        WidgetPointer getFocusedWidget() const;

    private:
        Viewport(ViewportID _id, ViewportRole _role, const Size& _size);

        void update(const LayoutContext& _layoutContext, const ViewportUpdate& _update);
        void clearInteraction();
        void invalidateLayout();
        bool stabilizeLayout(const LayoutContext& _layoutContext);
        void emitDrawCommands(const LayoutContext& _layoutContext, PaintContext& _paintContext);

        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        Size m_size = SizeZero;
        SurfaceRootPointer m_root = nullptr;
        ego::SharedPointer<InputRouter> m_inputRouter = nullptr;
    };

    EGO_POINTER(Viewport);
    EGO_WEAK_POINTER(Viewport);
} // namespace ego::gui
