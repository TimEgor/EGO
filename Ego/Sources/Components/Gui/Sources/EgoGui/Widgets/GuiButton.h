#pragma once

#include <functional>
#include <string>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiButton;
    EGO_POINTER(GuiButton);

    using GuiClickedHandler = std::function<GuiReply()>;

    class GuiButton final : public GuiWidget
    {
    public:
        GuiButton() = default;

        static GuiButtonPointer Create();

        void setText(std::string _text);
        const std::string& getText() const;

        void setOnClicked(GuiClickedHandler _handler);
        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiButton, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        std::string m_text;
        GuiClickedHandler m_onClicked;
        GuiMargin m_padding = GuiMargin(10.0f, 6.0f);
        GuiColor m_backgroundColor = GuiColor(0.16f, 0.18f, 0.20f, 1.0f);
        GuiColor m_textColor = GuiColorWhite;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };

} // namespace ego::gui
