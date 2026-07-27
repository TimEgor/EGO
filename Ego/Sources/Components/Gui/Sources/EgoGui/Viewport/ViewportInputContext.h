#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    class ViewportManager;
    class Window;

    EGO_POINTER(Window);

    class ViewportInputContext final : public NonCopyable
    {
    public:
        class ViewportManagerAccessor final : public NonInstanceable
        {
            friend class ViewportManager;

            static ViewportInputContext Create(ViewportManager& _manager, ViewportID _inputViewportID);
        };

        bool beginWindowDrag(const WindowPointer& _window, const Position& _screenPosition, const Rect& _localWindowBounds);
        void updateWindowDrag(const WindowPointer& _window, const Position& _screenPosition);
        bool finishWindowDrag(const WindowPointer& _window, const Position& _screenPosition);
        void cancelWindowDrag(const WindowPointer& _window);
        bool allowsInteractionOutsideSurface() const;

    private:
        ViewportInputContext(ViewportManager& _manager, ViewportID _inputViewportID);

        ViewportManager& m_manager;
        ViewportID m_inputViewportID = InvalidViewportID;
    };
} // namespace ego::gui
