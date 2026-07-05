#pragma once

#include <vector>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiVerticalBox;
    EGO_POINTER(GuiVerticalBox);

    class GuiBoxSlot final
    {
    public:
        GuiBoxSlot() = default;
        explicit GuiBoxSlot(const GuiWidgetPointer& _widget);

        GuiBoxSlot& setPadding(const GuiMargin& _padding);
        GuiBoxSlot& setHorizontalAlignment(GuiHorizontalAlignment _alignment);
        GuiBoxSlot& setSizePolicy(GuiSizePolicy _sizePolicy);
        GuiBoxSlot& setFill(float _fill);

        GuiWidgetPointer getWidget() const;
        const GuiMargin& getPadding() const;
        GuiHorizontalAlignment getHorizontalAlignment() const;
        GuiSizePolicy getSizePolicy() const;
        float getFill() const;

    private:
        GuiWidgetPointer m_widget = nullptr;
        GuiMargin m_padding;
        GuiHorizontalAlignment m_horizontalAlignment = GuiHorizontalAlignment::Stretch;
        GuiSizePolicy m_sizePolicy = GuiSizePolicy::Content;
        float m_fill = 1.0f;
    };

    class GuiVerticalBox final : public GuiWidget
    {
    public:
        using SlotCollection = std::vector<GuiBoxSlot>;

        GuiVerticalBox() = default;

        static GuiVerticalBoxPointer Create();

        GuiBoxSlot& addSlot(const GuiWidgetPointer& _widget);
        void clearSlots();
        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiVerticalBox, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        SlotCollection m_slots;
    };

} // namespace ego::gui
