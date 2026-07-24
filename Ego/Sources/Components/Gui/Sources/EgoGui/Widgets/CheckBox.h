#pragma once

#include <functional>
#include <string>

#include "EgoCore/Callback/StableCallback.h"

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class CheckBox;
    EGO_POINTER(CheckBox);

    using CheckedChangedHandler = std::function<void(bool)>;

    class CheckBox final : public Widget
    {
    public:
        static CheckBoxPointer Create();
        static CheckBoxPointer Create(std::string _text, bool _isChecked = false, CheckedChangedHandler _onChanged = {});

        void setText(std::string _text);
        const std::string& getText() const;

        void setChecked(bool _isChecked);
        bool isChecked() const;
        void onChange(CheckedChangedHandler _handler);

        EGO_RTTI_VIRTUAL(CheckBox, Widget);

    protected:
        InputReply onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event) override;
        InputReply onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event) override;
        void onPointerEnter(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerLeave(WidgetUpdateContext& _context, const Position& _position, const InputModifiers& _modifiers) override;
        void onPointerCaptureLost(WidgetUpdateContext& _context, const Position& _position) override;
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        CheckBox() = default;

        void applyUserChecked(bool _isChecked);

        std::string m_text;
        bool m_isChecked = false;
        StableCallback<bool> m_onCheckedChanged;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };

} // namespace ego::gui
