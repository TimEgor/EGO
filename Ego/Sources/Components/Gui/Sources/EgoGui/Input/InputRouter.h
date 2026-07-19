#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Input/Input.h"
#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class SurfaceRoot;

    class InputRouter final : public NonCopyable, public ego::EnableSharedFromThis<InputRouter>, private InputConsumer
    {
    public:
        ~InputRouter() override;

        static ego::SharedPointer<InputRouter> Create(const ego::SharedPointer<SurfaceRoot>& _root);

        void process(const InputEvent& _event);

        void refreshAfterLayout();
        void clear();

        WidgetPointer getFocusedWidget() const;

    private:
        class ProcessingScope final : public NonCopyable
        {
        public:
            explicit ProcessingScope(InputRouter& _router);
            ~ProcessingScope() override;

            explicit operator bool() const;

        private:
            ego::SharedPointer<InputRouter> m_router = nullptr;
            bool m_isActive = false;
        };

        explicit InputRouter(const ego::SharedPointer<SurfaceRoot>& _root);

        using MouseButtonMask = uint8_t;
        using WidgetPath = std::vector<WidgetPointer>;
        using WeakWidgetPath = std::vector<WidgetWeakPointer>;

        struct PointerState final
        {
            Position m_position = PositionZero;
            InputModifiers m_modifiers;
            WeakWidgetPath m_hoverPath;
            WidgetWeakPointer m_captureWidget;
            MouseButtonMask m_captureButtons = 0;
        };

        static MouseButtonMask GetMouseButtonMask(MouseInputKey _key);
        static bool ContainsWidget(const WidgetPath& _path, const WidgetPointer& _widget);

        void onPointerMove(const PointerMoveEvent& _event) override;
        void onPointerExit(const PointerExitEvent& _event) override;
        void onMouseButton(const MouseButtonEvent& _event) override;
        void onMouseWheel(const MouseWheelEvent& _event) override;
        void onKey(const KeyEvent& _event) override;
        void onTextInput(const TextInputEvent& _event) override;
        void onViewportDeactivated(const ViewportDeactivatedEvent& _event) override;

        WidgetPath buildPath(const WidgetPointer& _target) const;
        WidgetPointer takeInvalidTarget(WidgetWeakPointer& _target);
        bool applyReply(const WidgetPointer& _responder, InputReply _reply);
        void clearState();

        void setFocusedWidget(const WidgetPointer& _widget);
        void capturePointer(const WidgetPointer& _widget, MouseInputKey _key);
        void releasePointerButton(MouseInputKey _key);
        void clearPointerCapture();
        void updateHover(const WidgetPath& _path);

        ego::SharedPointer<SurfaceRoot> m_root = nullptr;
        WidgetWeakPointer m_focusedWidget;
        PointerState m_pointerState;
        bool m_isProcessing = false;
    };
} // namespace ego::gui
