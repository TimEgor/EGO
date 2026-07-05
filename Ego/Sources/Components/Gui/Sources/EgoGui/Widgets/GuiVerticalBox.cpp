#include "GuiVerticalBox.h"

#include <algorithm>

ego::gui::GuiBoxSlot::GuiBoxSlot(const GuiWidgetPointer& _widget)
    : m_widget(_widget)
{
}

ego::gui::GuiBoxSlot& ego::gui::GuiBoxSlot::setPadding(const GuiMargin& _padding)
{
    m_padding = _padding;
    return *this;
}

ego::gui::GuiBoxSlot& ego::gui::GuiBoxSlot::setHorizontalAlignment(GuiHorizontalAlignment _alignment)
{
    m_horizontalAlignment = _alignment;
    return *this;
}

ego::gui::GuiBoxSlot& ego::gui::GuiBoxSlot::setSizePolicy(GuiSizePolicy _sizePolicy)
{
    m_sizePolicy = _sizePolicy;
    return *this;
}

ego::gui::GuiBoxSlot& ego::gui::GuiBoxSlot::setFill(float _fill)
{
    m_fill = _fill;
    return *this;
}

ego::gui::GuiWidgetPointer ego::gui::GuiBoxSlot::getWidget() const
{
    return m_widget;
}

const ego::gui::GuiMargin& ego::gui::GuiBoxSlot::getPadding() const
{
    return m_padding;
}

ego::gui::GuiHorizontalAlignment ego::gui::GuiBoxSlot::getHorizontalAlignment() const
{
    return m_horizontalAlignment;
}

ego::gui::GuiSizePolicy ego::gui::GuiBoxSlot::getSizePolicy() const
{
    return m_sizePolicy;
}

float ego::gui::GuiBoxSlot::getFill() const
{
    return m_fill;
}

ego::gui::GuiVerticalBoxPointer ego::gui::GuiVerticalBox::Create()
{
    return new GuiVerticalBox();
}

ego::gui::GuiBoxSlot& ego::gui::GuiVerticalBox::addSlot(const GuiWidgetPointer& _widget)
{
    m_slots.emplace_back(_widget);
    return m_slots.back();
}

void ego::gui::GuiVerticalBox::clearSlots()
{
    m_slots.clear();
}

ego::gui::GuiReply ego::gui::GuiVerticalBox::handleEvent(const GuiInputEvent& _event)
{
    if (!isVisible())
    {
        return GuiReply::Unhandled();
    }

    for (SlotCollection::reverse_iterator iter = m_slots.rbegin(); iter != m_slots.rend(); ++iter)
    {
        const GuiWidgetPointer widget = iter->getWidget();
        if (!widget || !widget->isVisible())
        {
            continue;
        }

        const GuiReply reply = widget->handleEvent(_event);
        if (reply.m_isHandled)
        {
            return reply;
        }
    }

    return GuiReply::Unhandled();
}

ego::gui::GuiSize ego::gui::GuiVerticalBox::onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    GuiSize result = GuiSizeZero;

    for (const GuiBoxSlot& slot : m_slots)
    {
        const GuiWidgetPointer widget = slot.getWidget();
        if (!widget)
        {
            continue;
        }

        const GuiMargin& padding = slot.getPadding();
        const GuiSize childAvailableSize((std::max)(0.0f, _availableSize.m_x - padding.getHorizontal()), (std::max)(0.0f, _availableSize.m_y - padding.getVertical()));
        const GuiSize childDesiredSize = widget->measure(_context, childAvailableSize);

        result.m_x = (std::max)(result.m_x, childDesiredSize.m_x + padding.getHorizontal());
        result.m_y += childDesiredSize.m_y + padding.getVertical();
    }

    return result;
}

void ego::gui::GuiVerticalBox::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    float fixedHeight = 0.0f;
    float totalFill = 0.0f;

    for (const GuiBoxSlot& slot : m_slots)
    {
        if (!slot.getWidget())
        {
            continue;
        }

        const GuiMargin& padding = slot.getPadding();
        if (slot.getSizePolicy() == GuiSizePolicy::Fill)
        {
            totalFill += slot.getFill();
        }
        else
        {
            fixedHeight += slot.getWidget()->getDesiredSize().m_y + padding.getVertical();
        }
    }

    const float fillHeight = (std::max)(0.0f, _rect.m_size.m_y - fixedHeight);
    float currentY = _rect.m_position.m_y;

    for (const GuiBoxSlot& slot : m_slots)
    {
        const GuiWidgetPointer widget = slot.getWidget();
        if (!widget)
        {
            continue;
        }

        const GuiMargin& padding = slot.getPadding();
        const float availableWidth = (std::max)(0.0f, _rect.m_size.m_x - padding.getHorizontal());
        float childHeight = widget->getDesiredSize().m_y;
        if (slot.getSizePolicy() == GuiSizePolicy::Fill && totalFill > 0.0f)
        {
            childHeight = fillHeight * (slot.getFill() / totalFill) - padding.getVertical();
        }

        float childWidth = widget->getDesiredSize().m_x;
        float childX = _rect.m_position.m_x + padding.m_left;
        if (slot.getHorizontalAlignment() == GuiHorizontalAlignment::Stretch)
        {
            childWidth = availableWidth;
        }
        else if (slot.getHorizontalAlignment() == GuiHorizontalAlignment::Center)
        {
            childX += (availableWidth - childWidth) * 0.5f;
        }
        else if (slot.getHorizontalAlignment() == GuiHorizontalAlignment::Right)
        {
            childX += availableWidth - childWidth;
        }

        const GuiRect childRect(childX, currentY + padding.m_top, (std::max)(0.0f, childWidth), (std::max)(0.0f, childHeight));
        widget->arrange(_context, childRect);
        currentY += childRect.m_size.m_y + padding.getVertical();
    }
}

void ego::gui::GuiVerticalBox::onPaint(GuiPaintContext& _context) const
{
    for (const GuiBoxSlot& slot : m_slots)
    {
        const GuiWidgetPointer widget = slot.getWidget();
        if (widget)
        {
            widget->paint(_context);
        }
    }
}
