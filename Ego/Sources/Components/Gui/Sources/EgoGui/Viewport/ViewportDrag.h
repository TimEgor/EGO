#pragma once

#include "ViewportManager.h"

namespace ego::gui
{
    class ViewportManager::ViewportDrag final
    {
    public:
        bool begin(
            ViewportManager& _manager,
            ViewportID _inputViewportID,
            const WindowPointer& _window,
            const Position& _screenPosition,
            const Rect& _localWindowBounds);
        void update(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
        bool finish(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition);
        void cancel(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window);
        bool allowsInteractionOutsideSurface(const ViewportManager& _manager, ViewportID _inputViewportID) const;
        void completeInput(ViewportManager& _manager, ViewportID _inputViewportID);

        void handleViewportDestroying(ViewportManager& _manager, const ViewportPointer& _viewport);
        bool isValid(const ViewportManager& _manager) const;
        bool isInput(ViewportID _inputViewportID, const WindowPointer& _window) const;
        void reset(ViewportManager& _manager, bool _clearInputInteraction = true);

    private:
        void updateTarget(ViewportManager& _manager);
        void clearTarget(ViewportManager& _manager);
        bool commit(ViewportManager& _manager);
        bool transferWindow(ViewportManager& _manager, const ViewportPointer& _targetViewport);
        bool createWindowViewport(ViewportManager& _manager);
        bool createDetachedViewport(ViewportManager& _manager);

        ViewportID m_inputViewportID = InvalidViewportID;
        ViewportID m_windowViewportID = InvalidViewportID;
        ViewportID m_targetViewportID = InvalidViewportID;
        WindowPointer m_window = nullptr;
        Rect m_screenWindowBounds;
        Position m_grabOffset = PositionZero;
        Position m_screenPosition = PositionZero;
        bool m_isMovingViewport = false;
        bool m_isWindowViewportCreationPending = false;
        bool m_isCompletionPending = false;
        bool m_isCommitting = false;
    };
} // namespace ego::gui
