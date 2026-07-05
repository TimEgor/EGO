#pragma once

#include <string>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiCheckBox;
    EGO_POINTER(GuiCheckBox);

    class GuiCheckBox final : public GuiWidget
    {
    public:
        GuiCheckBox() = default;

        static GuiCheckBoxPointer Create();

        void setText(std::string _text);
        const std::string& getText() const;

        void setChecked(bool _isChecked);
        bool isChecked() const;

        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiCheckBox, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        std::string m_text;
        bool m_isChecked = false;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };

} // namespace ego::gui
