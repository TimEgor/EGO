#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Layout/Layout.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    class Viewport;
    class ViewportInputContext;
    class ViewportProvider;
    class Window;
    struct GuiRenderData;
    class InputEvent;
    struct ViewportUpdate;

    EGO_POINTER(Viewport);
    EGO_POINTER(ViewportProvider);
    EGO_POINTER(Window);

    class ViewportManager final : public NonCopyable
    {
    public:
        class ViewportInputAccessor final : public NonInstanceable
        {
            friend class ViewportInputContext;

            static bool BeginWindowDrag(
                ViewportManager& _manager,
                ViewportID _inputViewportID,
                const WindowPointer& _window,
                const Position& _screenPosition,
                const Rect& _localWindowBounds);
            static void UpdateWindowDrag(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
            static bool FinishWindowDrag(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
            static void CancelWindowDrag(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window);
            static bool AllowsInteractionOutsideSurface(const ViewportManager& _manager, ViewportID _inputViewportID);
        };

        ViewportManager();
        ~ViewportManager() override;

        bool init(const ViewportProviderPointer& _provider, bool _enableMultiViewport);
        void release();

        bool update(const LayoutContext& _layoutContext);
        GuiRenderData buildFrame(const LayoutContext& _layoutContext);

        ViewportPointer createViewport(const ViewportDesc& _desc);
        bool destroyViewport(const ViewportPointer& _viewport);

        ViewportPointer getPrimaryViewport() const;
        ViewportPointer findViewport(const WindowPointer& _window) const;

        void setMultiViewportEnabled(bool _isEnabled);
        bool isMultiViewportEnabled() const;
        void invalidateLayouts();

    private:
        class ViewportDrag;

        static constexpr ViewportID FirstViewportID = 1;
        static constexpr float DefaultManagedViewportExtent = 500.0f;

        struct ViewportRecord final
        {
            ViewportPointer m_viewport = nullptr;
            bool m_isManaged = false;
        };

        using ViewportCollection = std::vector<ViewportRecord>;

        ViewportPointer createViewport(ViewportRole _role, const ViewportDesc& _desc, bool _isManaged);
        bool destroyViewport(const ViewportPointer& _viewport, bool _recoverWindows);
        void destroyViewports();
        size_t findViewportIndex(ViewportID _viewportID) const;
        ViewportPointer findViewport(ViewportID _viewportID) const;
        bool containsViewport(const ViewportPointer& _viewport) const;
        bool isManagedViewport(const ViewportPointer& _viewport) const;

        void applyManagedViewportUpdate(const ViewportPointer& _viewport, const ViewportUpdate& _update);
        bool shouldDestroyManagedViewport(const ViewportPointer& _viewport) const;
        void updateWindowViewports();
        void mergeManagedViewports();
        void syncManagedViewport(const ViewportPointer& _viewport);
        bool createWindowViewport(const WindowPointer& _window, const ViewportPointer& _sourceViewport, const Rect& _screenBounds);
        void recoverViewportWindows(const ViewportPointer& _viewport);
        Rect getWindowScreenBounds(const WindowPointer& _window, const ViewportPointer& _viewport) const;

        bool beginWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition, const Rect& _localWindowBounds);
        void updateWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
        bool finishWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
        void cancelWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window);
        bool allowsInteractionOutsideSurface(ViewportID _inputViewportID) const;
        void completeSurfaceInput(ViewportID _inputViewportID);
        bool isWindowDragInput(ViewportID _inputViewportID, const WindowPointer& _window) const;
        void validateWindowDrag();

        ViewportProviderPointer m_provider = nullptr;
        ViewportCollection m_viewports;
        ViewportPointer m_primaryViewport = nullptr;
        std::unique_ptr<ViewportDrag> m_viewportDrag;
        ViewportID m_nextViewportID = FirstViewportID;
        bool m_isMultiViewportEnabled = false;
    };
} // namespace ego::gui
