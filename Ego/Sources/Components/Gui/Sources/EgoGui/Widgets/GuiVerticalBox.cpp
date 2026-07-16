#include "GuiVerticalBox.h"

#include <algorithm>

ego::gui::GuiBoxLayout ego::gui::GuiBoxLayout::Content(const GuiMargin& _padding, GuiHorizontalAlignment _horizontalAlignment)
{
    GuiBoxLayout layout;
    layout.m_padding = _padding;
    layout.m_horizontalAlignment = _horizontalAlignment;
    return layout;
}

ego::gui::GuiBoxLayout ego::gui::GuiBoxLayout::Fill(float _weight, const GuiMargin& _padding, GuiHorizontalAlignment _horizontalAlignment)
{
    GuiBoxLayout layout;
    layout.m_padding = _padding;
    layout.m_horizontalAlignment = _horizontalAlignment;
    layout.m_fillWeight = (std::max)(0.0f, _weight);
    return layout;
}

ego::gui::GuiVerticalBoxPointer ego::gui::GuiVerticalBox::Create()
{
    return new GuiVerticalBox();
}

bool ego::gui::GuiVerticalBox::addChild(const GuiWidgetPointer& _widget, const GuiBoxLayout& _layout)
{
    if (!GuiContainer::addChild(_widget))
    {
        return false;
    }

    m_layouts.push_back(_layout);
    return true;
}

void ego::gui::GuiVerticalBox::clearChildren()
{
    GuiContainer::clearChildren();
    m_layouts.clear();
}

ego::gui::GuiSize ego::gui::GuiVerticalBox::onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    GuiSize result = GuiSizeZero;
    const ChildCollection& children = getChildren();

    for (size_t childIndex = 0; childIndex < children.size(); ++childIndex)
    {
        const GuiWidgetPointer& widget = children[childIndex];
        if (!widget)
        {
            continue;
        }
        if (widget->isCollapsed())
        {
            widget->measure(_context, GuiSizeZero);
            continue;
        }

        const GuiMargin& padding = m_layouts[childIndex].m_padding;
        const GuiSize childAvailableSize((std::max)(0.0f, _availableSize.m_x - padding.getHorizontal()), (std::max)(0.0f, _availableSize.m_y - padding.getVertical()));
        const GuiSize childDesiredSize = widget->measure(_context, childAvailableSize);

        const float slotHeight = childDesiredSize.m_y + padding.getVertical();
        result.m_x = (std::max)(result.m_x, childDesiredSize.m_x + padding.getHorizontal());
        result.m_y += slotHeight;
    }

    return result;
}

void ego::gui::GuiVerticalBox::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    float fixedHeight = 0.0f;
    float totalFillWeight = 0.0f;
    const ChildCollection& children = getChildren();

    for (size_t childIndex = 0; childIndex < children.size(); ++childIndex)
    {
        const GuiWidgetPointer& widget = children[childIndex];
        if (!widget)
        {
            continue;
        }
        if (widget->isCollapsed())
        {
            widget->arrange(_context, GuiRect(_rect.m_position, GuiSizeZero));
            continue;
        }

        const GuiBoxLayout& layout = m_layouts[childIndex];
        fixedHeight += layout.m_padding.getVertical();
        if (layout.m_fillWeight > 0.0f)
        {
            totalFillWeight += layout.m_fillWeight;
        }
        else
        {
            fixedHeight += widget->getDesiredSize().m_y;
        }
    }

    const float fillHeight = (std::max)(0.0f, _rect.m_size.m_y - fixedHeight);
    float currentY = _rect.m_position.m_y;
    for (size_t childIndex = 0; childIndex < children.size(); ++childIndex)
    {
        const GuiWidgetPointer& widget = children[childIndex];
        if (!widget || widget->isCollapsed())
        {
            continue;
        }

        const GuiBoxLayout& layout = m_layouts[childIndex];
        const GuiMargin& padding = layout.m_padding;
        const float availableWidth = (std::max)(0.0f, _rect.m_size.m_x - padding.getHorizontal());

        float childHeight = widget->getDesiredSize().m_y;
        if (layout.m_fillWeight > 0.0f && totalFillWeight > 0.0f)
        {
            childHeight = fillHeight * layout.m_fillWeight / totalFillWeight;
        }

        float childWidth = (std::min)(widget->getDesiredSize().m_x, availableWidth);
        float childX = _rect.m_position.m_x + padding.m_left;
        if (layout.m_horizontalAlignment == GuiHorizontalAlignment::Stretch)
        {
            childWidth = availableWidth;
        }
        else if (layout.m_horizontalAlignment == GuiHorizontalAlignment::Center)
        {
            childX += (availableWidth - childWidth) * 0.5f;
        }
        else if (layout.m_horizontalAlignment == GuiHorizontalAlignment::Right)
        {
            childX += availableWidth - childWidth;
        }

        const GuiRect childRect(childX, currentY + padding.m_top, (std::max)(0.0f, childWidth), (std::max)(0.0f, childHeight));
        widget->arrange(_context, childRect);
        currentY += padding.m_top + childRect.m_size.m_y + padding.m_bottom;
    }
}
