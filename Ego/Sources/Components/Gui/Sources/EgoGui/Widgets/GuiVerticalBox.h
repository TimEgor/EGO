#pragma once

#include <cstddef>
#include <vector>

#include "EgoGui/Widgets/GuiContainer.h"

namespace ego::gui
{
    class GuiVerticalBox;
    EGO_POINTER(GuiVerticalBox);

    class GuiBoxLayout final
    {
    public:
        GuiBoxLayout() = default;

        static GuiBoxLayout Content(const GuiMargin& _padding = GuiMargin(), GuiHorizontalAlignment _horizontalAlignment = GuiHorizontalAlignment::Stretch);
        static GuiBoxLayout Fill(float _weight = 1.0f, const GuiMargin& _padding = GuiMargin(), GuiHorizontalAlignment _horizontalAlignment = GuiHorizontalAlignment::Stretch);

    private:
        friend class GuiVerticalBox;

        GuiMargin m_padding;
        GuiHorizontalAlignment m_horizontalAlignment = GuiHorizontalAlignment::Stretch;
        float m_fillWeight = 0.0f;
    };

    class GuiVerticalBox final : public GuiContainer
    {
    public:
        GuiVerticalBox() = default;

        static GuiVerticalBoxPointer Create();

        bool addChild(const GuiWidgetPointer& _widget, const GuiBoxLayout& _layout = GuiBoxLayout());
        void clearChildren();

        EGO_RTTI_VIRTUAL(GuiVerticalBox, GuiContainer);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;

    private:
        using LayoutCollection = std::vector<GuiBoxLayout>;

        LayoutCollection m_layouts;
    };

} // namespace ego::gui
