#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Input/Input.h"
#include "EgoGui/Widgets/Window.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    class InputRouter;
    class SurfaceRoot;
    class WindowHost;
    struct ViewportUpdate;

    EGO_POINTER(SurfaceRoot);
    EGO_POINTER(WindowHost);

    class Viewport final
        : public NonCopyable
    {
    public:
        class ViewportAccessor final
            : public NonInstanceable
        {
        public:
            static ego::SharedPointer<Viewport> Create(ViewportID _id, ViewportRole _role, const Size& _size);
            static void Update(Viewport& _viewport, const LayoutContext& _layoutContext, const ViewportUpdate& _update);
            static void ClearInteraction(Viewport& _viewport);
            static void InvalidateLayout(Viewport& _viewport);
            static void EmitDrawCommands(Viewport& _viewport, const LayoutContext& _layoutContext, PaintContext& _paintContext);
        };

        using WidgetCollection = std::vector<WidgetPointer>;

        ~Viewport() override;

        ViewportID getID() const;
        ViewportRole getRole() const;
        const Size& getSize() const;
        GraphicPresenterPointer getGraphicPresenterPointer() const;

        bool add(const WidgetPointer& _widget);
        bool addWindow(const WindowPointer& _window, const WindowPlacement& _placement = WindowPlacement());
        WidgetPointer remove(const WidgetPointer& _widget);
        void clear();
        const WidgetCollection& getWidgets() const;
        WidgetPointer getFocusedWidget() const;

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultDockingSpaceID() const;
        DockingSpaceID getWindowDockingSpaceID(const WindowPointer& _window) const;
        bool moveWindow(const WindowPointer& _window, const WindowPlacement& _placement);

    private:
        Viewport(ViewportID _id, ViewportRole _role, const Size& _size);

        WindowHostPointer getWindowHost() const;
        void setSize(const Size& _size);
        void update(const LayoutContext& _layoutContext, const ViewportUpdate& _update);
        void clearInteraction();
        void invalidateLayout();
        bool stabilizeLayout(const LayoutContext& _layoutContext);
        void emitDrawCommands(const LayoutContext& _layoutContext, PaintContext& _paintContext);

        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        Size m_size = SizeZero;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        SurfaceRootPointer m_root = nullptr;
        ego::SharedPointer<InputRouter> m_inputRouter = nullptr;
    };

    EGO_POINTER(Viewport);
    EGO_WEAK_POINTER(Viewport);
} // namespace ego::gui
