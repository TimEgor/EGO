#pragma once

#include <string>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiTextBlock;
    EGO_POINTER(GuiTextBlock);

    class GuiTextBlock final : public GuiWidget
    {
    public:
        GuiTextBlock() = default;

        static GuiTextBlockPointer Create();

        void setText(std::string _text);
        const std::string& getText() const;

        void setColor(const GuiColor& _color);
        const GuiColor& getColor() const;

        EGO_RTTI_VIRTUAL(GuiTextBlock, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        std::string m_text;
        GuiColor m_color = GuiColorWhite;
    };

} // namespace ego::gui
