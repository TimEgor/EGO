#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"

#include "EgoGui/Input/Input.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class InputContext;
    class InputPass;
    class SurfaceRoot;
    class ViewportInputContext;
    class Window;
    class WindowDrag;

    EGO_POINTER(Window);

    class InputState final : public NonCopyable
    {
    public:
        class InputPassAccessor final : public NonInstanceable
        {
            friend class InputPass;

            static bool BeginDispatch(InputState& _state);
            static void EndDispatch(InputState& _state);
            static bool IsClearPending(const InputState& _state);
            static bool BeginClear(InputState& _state);
            static void EndClear(InputState& _state);
            static bool BeginRefresh(InputState& _state);
            static void EndRefresh(InputState& _state);

            static void UpdatePointer(InputState& _state, const Position& _position, const Position& _screenPosition, const InputModifiers& _modifiers);
            static WidgetPointer GetPointerCapture(const InputState& _state);
            static void ResetPointerCaptureIfOwned(InputState& _state, const WidgetPointer& _widget);
            static WidgetPointer FindWidgetAt(const InputState& _state, const Position& _position);
            static bool IsInputTarget(const InputState& _state, const WidgetPointer& _widget);
            static std::vector<WidgetPointer> BuildPath(const InputState& _state, const WidgetPointer& _target);
            static void UpdateHover(InputState& _state, const std::vector<WidgetPointer>& _path);
            static void SetFocusedWidget(InputState& _state, const WidgetPointer& _widget);
            static bool CapturePointer(InputState& _state, const WidgetPointer& _widget, MouseInputKey _key);
            static void ReleasePointerButton(InputState& _state, MouseInputKey _key);
            static WindowPointer ClearPointerCapture(InputState& _state);
            static WindowPointer ApplyPointerCaptureCancellation(InputState& _state);
            static WindowPointer ClearState(InputState& _state);
            static WindowPointer RefreshState(InputState& _state);
        };

        class WindowDragAccessor final : public NonInstanceable
        {
            friend class WindowDrag;

            static bool TransferPointerCapture(InputState& _state, const WidgetPointer& _source, const WidgetPointer& _target);
        };

        explicit InputState(SurfaceRoot& _root);
        ~InputState() override;

        void dispatch(const InputEvent& _event, ViewportInputContext& _viewportInput);
        void refresh();
        void clear();
        void detachInputState(const WidgetPointer& _subtree = nullptr);
        void requestPointerCaptureCancellation(const WidgetPointer& _relatedWidget = nullptr);
        void flushPointerCaptureCancellation();
        void cancelWindowDrag(const WindowPointer& _window);

        WidgetPointer getFocusedWidget() const;
        bool isWindowDragActive(const WindowPointer& _window) const;
        WindowPointer getDraggedWindow() const;

    private:
        using MouseButtonMask = uint8_t;
        using WidgetPath = std::vector<WidgetPointer>;
        using WeakWidgetPath = std::vector<WidgetWeakPointer>;

        struct PointerState final
        {
            Position m_position = PositionZero;
            Position m_screenPosition = PositionZero;
            InputModifiers m_modifiers;
            WeakWidgetPath m_hoverPath;
            WidgetWeakPointer m_captureWidget;
            MouseButtonMask m_captureButtons = 0;
        };

        static MouseButtonMask GetMouseButtonMask(MouseInputKey _key);
        static bool ContainsDirectChild(const Widget& _parent, const Widget& _child);

        bool transferPointerCapture(const WidgetPointer& _source, const WidgetPointer& _target);
        WidgetPointer findWidgetAt(const Position& _position) const;
        bool isInputTarget(const WidgetPointer& _widget) const;
        WidgetPath buildPath(const WidgetPointer& _target) const;
        void setFocusedWidget(const WidgetPointer& _widget);
        bool capturePointer(const WidgetPointer& _widget, MouseInputKey _key);
        void releasePointerButton(MouseInputKey _key);
        WindowPointer clearPointerCapture();
        WindowPointer applyPointerCaptureCancellation();
        WindowPointer clearState();
        WindowPointer refreshState();
        void updateHover(const WidgetPath& _path);

        SurfaceRoot& m_root;
        std::unique_ptr<WindowDrag> m_windowDrag;
        WidgetWeakPointer m_focusedWidget;
        PointerState m_pointer;
        WidgetWeakPointer m_pointerCaptureCancellationTarget;
        size_t m_dispatchDepth = 0;
        bool m_isClearPending = false;
        bool m_isDispatchingInput = false;
    };
} // namespace ego::gui
