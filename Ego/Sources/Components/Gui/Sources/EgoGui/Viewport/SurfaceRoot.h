#pragma once

#include <vector>

#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoGui/Docking/DockingTypes.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Container.h"

namespace ego::gui
{
    class DockingArea;
    class DockingOverlay;
    class SurfaceRoot;
    class Viewport;
    class InputContext;
    class InputPass;
    class InputState;
    class Window;
    class WindowDrag;
    struct DockingPreview;

    EGO_POINTER(DockingArea);
    EGO_POINTER(DockingOverlay);
    EGO_POINTER(SurfaceRoot);
    EGO_POINTER(Window);
    EGO_WEAK_POINTER(Window);

    class SurfaceRoot final : public Container
    {
    public:
        class ViewportAccessor final : public NonInstanceable
        {
            friend class Viewport;

            static SurfaceRootPointer Create();
            static bool AttachWindow(SurfaceRoot& _root, const WindowPointer& _window, bool _isBound);
            static WindowPointer ExtractWindow(SurfaceRoot& _root, const WindowPointer& _window);
            static bool TransferWindowTo(SurfaceRoot& _root, SurfaceRoot& _target, const WindowPointer& _window);
            static void ClearWindows(SurfaceRoot& _root);
            static bool ContainsWindow(const SurfaceRoot& _root, const WindowPointer& _window);
            static size_t GetWindowCount(const SurfaceRoot& _root);
            static WindowPointer GetWindow(const SurfaceRoot& _root, size_t _index);
            static size_t GetFloatingWindowCount(const SurfaceRoot& _root);
            static WindowPointer GetFloatingWindow(const SurfaceRoot& _root, size_t _index);
            static bool IsWindowBound(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool IsWindowFloating(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool SetDockingEnabled(SurfaceRoot& _root, bool _isEnabled);
            static bool IsDockingEnabled(const SurfaceRoot& _root);
            static DockingSpaceID GetDefaultDockingSpaceID(const SurfaceRoot& _root);
            static DockingSpaceID GetWindowDockingSpaceID(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool DockWindow(SurfaceRoot& _root, const WindowPointer& _window, const DockingLocation& _location);
            static void InvalidateLayout(SurfaceRoot& _root);
            static void UpdateExternalDragPreview(SurfaceRoot& _root, const WindowPointer& _window, const Position& _position);
            static bool DockExternalWindow(SurfaceRoot& _root, const WindowPointer& _window, const Position& _position);
            static void ClearExternalDragPreview(SurfaceRoot& _root);
        };

        class InputAccessor final : public NonInstanceable
        {
            friend class InputContext;
            friend class InputPass;
            friend class InputState;

            static void ClearInteraction(SurfaceRoot& _root);
            static void ClearPreview(SurfaceRoot& _root);
            static void BringWidgetToFront(SurfaceRoot& _root, const WidgetPointer& _widget);
        };

        class WindowDragAccessor final : public NonInstanceable
        {
            friend class WindowDrag;

            static bool ContainsWindow(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool IsWindowFloating(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool IsWindowDocked(const SurfaceRoot& _root, const WindowPointer& _window);
            static bool MakeWindowFloating(SurfaceRoot& _root, const WindowPointer& _window, const Rect& _bounds);
            static const DockingStyle& GetDockingStyle(const SurfaceRoot& _root);
            static DockingPreview BuildPreview(const SurfaceRoot& _root, const WindowPointer& _window, const Position& _position);
            static void ShowPreview(SurfaceRoot& _root, const DockingPreview& _preview);
            static bool ApplyPreview(SurfaceRoot& _root, const WindowPointer& _window, const DockingPreview& _preview);
            static void ClearPreview(SurfaceRoot& _root);
        };

        ~SurfaceRoot() override;

        EGO_RTTI_VIRTUAL(SurfaceRoot, Container);

    private:
        using WindowCollection = std::vector<WindowPointer>;
        using WindowWeakCollection = std::vector<WindowWeakPointer>;

        SurfaceRoot() = default;

        static SurfaceRootPointer Create();

        bool initialize();
        bool attachWindow(const WindowPointer& _window, bool _isBound);
        WindowPointer extractWindow(const WindowPointer& _window);
        bool transferWindowTo(SurfaceRoot& _target, const WindowPointer& _window);
        void clearWindows();
        bool containsWindow(const WindowPointer& _window) const;
        size_t getWindowCount() const;
        WindowPointer getWindow(size_t _index) const;
        size_t getFloatingWindowCount() const;
        WindowPointer getFloatingWindow(size_t _index) const;
        bool isWindowBound(const WindowPointer& _window) const;
        bool isWindowFloating(const WindowPointer& _window) const;

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultDockingSpaceID() const;
        DockingSpaceID getWindowDockingSpaceID(const WindowPointer& _window) const;
        bool dockWindow(const WindowPointer& _window, const DockingLocation& _location);
        bool dockWindowToRoot(const WindowPointer& _window, DockingPlacement _placement, float _ratio);
        bool makeWindowFloating(const WindowPointer& _window, const Rect& _bounds);
        void clearInteraction();

        DockingPreview buildDockingPreview(const WindowPointer& _window, const Position& _position) const;
        void showDockingPreview(const DockingPreview& _preview);
        bool applyDockingPreview(const WindowPointer& _window, const DockingPreview& _preview);
        void clearDockingPreview();
        void updateExternalDragPreview(const WindowPointer& _window, const Position& _position);
        bool dockExternalWindow(const WindowPointer& _window, const Position& _position);
        void clearExternalDragPreview();

        void bringWidgetToFront(const WidgetPointer& _widget);

        bool attachFloatingWindow(const WindowPointer& _window);
        bool detachFloatingWindow(const WindowPointer& _window);
        void moveFloatingWindowToFront(const WindowPointer& _window);
        void removeWindowBinding(const WindowPointer& _window);

        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void updateGeometry(const LayoutContext& _context) override;
        size_t getChildCount() const override;
        WidgetPointer getChild(size_t _index) const override;

        DockingAreaPointer m_dockingArea = nullptr;
        WindowCollection m_floatingWindows;
        WindowWeakCollection m_boundWindows;
        DockingOverlayPointer m_dockingOverlay = nullptr;
        DockingStyle m_dockingStyle;
    };
} // namespace ego::gui
