#pragma once

#include <cstddef>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Input/WidgetUpdateContext.h"
#include "EgoGui/Widgets/Window.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    class SurfaceRoot;
    struct ViewportUpdate;

    EGO_POINTER(SurfaceRoot);

    class Viewport final
        : public NonCopyable
    {
    public:
        using WindowCollection = std::vector<WindowPointer>;

        ~Viewport() override;

        static ego::SharedPointer<Viewport> Create(ViewportID _id, ViewportRole _role, const Size& _size);

        ViewportID getID() const;
        ViewportRole getRole() const;
        const Size& getSize() const;
        GraphicPresenterPointer getGraphicPresenterPointer() const;
        WidgetPointer getRootWidget() const;

        bool addWindow(const WindowPointer& _window, const WindowPlacement& _placement = WindowPlacement());
        WindowPointer removeWindow(const WindowPointer& _window);
        void clear();
        WindowCollection getWindows() const;
        WidgetPointer getFocusedWidget() const;

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultDockingSpaceID() const;
        DockingSpaceID getWindowDockingSpaceID(const WindowPointer& _window) const;
        bool moveWindow(const WindowPointer& _window, const WindowPlacement& _placement);

        void update(const LayoutContext& _layoutContext, const ViewportUpdate& _update);
        void clearInteraction();
        void invalidateLayout();
        void emitDrawCommands(const LayoutContext& _layoutContext, PaintContext& _paintContext);

    private:
        Viewport(ViewportID _id, ViewportRole _role, const Size& _size);

        void setSize(const Size& _size);
        bool stabilize(const LayoutContext& _layoutContext, WidgetUpdateContext& _updateContext);
        WidgetUpdateContext createUpdateContext();

        static constexpr size_t MaximumLayoutPassCount = 64;

        ViewportID m_id = InvalidViewportID;
        ViewportRole m_role = ViewportRole::Secondary;
        Size m_size = SizeZero;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        SurfaceRootPointer m_root = nullptr;
        mutable WidgetUpdateState m_updateState;
    };

    EGO_POINTER(Viewport);
    EGO_WEAK_POINTER(Viewport);
} // namespace ego::gui
