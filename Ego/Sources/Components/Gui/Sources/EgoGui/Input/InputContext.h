#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGui/Core/Geometry.h"

#include "WindowDrag.h"

namespace ego::gui
{
    class InputPass;
    class InputState;
    class Window;

    EGO_POINTER(Window);

    class InputContext final : public NonCopyable
    {
    public:
        class InputAccessor final : public NonInstanceable
        {
            friend class InputPass;
            friend class InputState;

            static InputContext Create(WindowDrag& _windowDrag, const WindowDragContext& _windowDragContext, const Position& _screenPosition);
            static bool UpdateCapturedWindowDrag(InputContext& _context, const Position& _position);
            static bool FinishCapturedWindowDrag(InputContext& _context, const Position& _position);
            static void CancelWindowDragAfterCaptureFailure(InputContext& _context);
        };

        const Rect& getSurfaceBounds() const;
        void beginFloatingWindowDrag(const WindowPointer& _window, const Position& _position);
        void beginDockedWindowDrag(const WindowPointer& _window, const Position& _position);
        bool updateWindowDrag(const WindowPointer& _window, const Position& _position);
        void finishWindowDrag(const WindowPointer& _window, const Position& _position);
        void cancelWindowDrag(const WindowPointer& _window);
        bool allowsInteractionOutsideSurface() const;

    private:
        InputContext(WindowDrag& _windowDrag, const WindowDragContext& _windowDragContext, const Position& _screenPosition);

        bool hasCapturedWindowDrag() const;
        bool updateCapturedWindowDrag(const Position& _position);
        bool finishCapturedWindowDrag(const Position& _position);
        void cancelWindowDragAfterCaptureFailure();

        WindowDrag& m_windowDrag;
        WindowDragContext m_windowDragContext;
        const Position& m_screenPosition;
    };
} // namespace ego::gui
