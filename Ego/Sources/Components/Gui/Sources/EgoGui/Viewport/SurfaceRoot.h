#pragma once

#include <vector>

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingArea;
    class DockingOverlay;
    class SurfaceRoot;
    class Window;

    EGO_POINTER(DockingArea);
    EGO_POINTER(DockingOverlay);
    EGO_POINTER(SurfaceRoot);
    EGO_POINTER(Window);

    class SurfaceRoot final
        : public Container
    {
    public:
        using WindowCollection = std::vector<WindowPointer>;

        ~SurfaceRoot() override;

        static SurfaceRootPointer Create();

        bool addWindow(const WindowPointer& _window);
        WindowPointer removeWindow(const WindowPointer& _window);
        void clearWindows();
        WindowCollection getWindows() const;

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultDockingSpaceID() const;
        DockingSpaceID getWindowDockingSpaceID(const WindowPointer& _window) const;
        bool dockWindow(const WindowPointer& _window, const WindowPlacement& _placement);
        bool dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _ratio);
        bool makeWindowFloating(const WindowPointer& _window, const Rect& _bounds);

        DockingAreaPointer getDockingArea() const;
        DockingOverlayPointer getDockingOverlay() const;

        void bringWidgetToFront(const WidgetPointer& _widget);
        WidgetPointer findWidgetAt(const Position& _position);
        bool isInputTarget(const WidgetPointer& _widget) const;

        bool updateLayoutIfNeeded(const LayoutContext& _context, const Size& _size);

        EGO_RTTI_VIRTUAL(SurfaceRoot, Container);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;

    private:
        SurfaceRoot() = default;

        bool initialize();
        bool isHostedWindow(const WindowPointer& _window) const;
        bool attachFloatingWindow(const WindowPointer& _window);
        bool detachFloatingWindow(const WindowPointer& _window);
        void moveFloatingWindowToFront(const WindowPointer& _window);
        bool containsDirectChild(const Widget& _parent, const Widget& _child) const;

        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        DockingAreaPointer m_dockingArea = nullptr;
        WindowCollection m_floatingWindows;
        DockingOverlayPointer m_dockingOverlay = nullptr;
    };
} // namespace ego::gui
