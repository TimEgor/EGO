#pragma once

#include <functional>
#include <string>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class Button;
    EGO_POINTER(Button);

    using ClickedHandler = std::function<void()>;

    class Button final : public Widget
    {
    public:
        static ButtonPointer Create();
        static ButtonPointer Create(std::string _text, ClickedHandler _onClicked = {});

        void setText(std::string _text);
        const std::string& getText() const;

        void onClick(ClickedHandler _handler);

        EGO_RTTI_VIRTUAL(Button, Widget);

    protected:
        InputReply onPointerMove(InputContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(InputContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(const Position& _position) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        Button() = default;

        std::string m_text;
        StableCallback<> m_onClicked;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };

} // namespace ego::gui
