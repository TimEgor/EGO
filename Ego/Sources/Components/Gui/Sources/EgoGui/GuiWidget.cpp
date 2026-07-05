#include "GuiWidget.h"

ego::gui::GuiSize ego::gui::GuiWidget::measure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    if (isCollapsed())
    {
        m_desiredSize = GuiSizeZero;
        return m_desiredSize;
    }

    m_desiredSize = onMeasure(_context, _availableSize);
    return m_desiredSize;
}

void ego::gui::GuiWidget::arrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    m_rect = _rect;
    if (!isCollapsed())
    {
        onArrange(_context, _rect);
    }
}

void ego::gui::GuiWidget::paint(GuiPaintContext& _context) const
{
    if (!isVisible())
    {
        return;
    }

    onPaint(_context);
}

ego::gui::GuiReply ego::gui::GuiWidget::handleEvent(const GuiInputEvent&)
{
    return GuiReply::Unhandled();
}

void ego::gui::GuiWidget::setVisibility(GuiVisibility _visibility)
{
    m_visibility = _visibility;
}

ego::gui::GuiVisibility ego::gui::GuiWidget::getVisibility() const
{
    return m_visibility;
}

bool ego::gui::GuiWidget::isVisible() const
{
    return m_visibility == GuiVisibility::Visible;
}

bool ego::gui::GuiWidget::isCollapsed() const
{
    return m_visibility == GuiVisibility::Collapsed;
}

const ego::gui::GuiSize& ego::gui::GuiWidget::getDesiredSize() const
{
    return m_desiredSize;
}

const ego::gui::GuiRect& ego::gui::GuiWidget::getRect() const
{
    return m_rect;
}

ego::gui::GuiSize ego::gui::GuiWidget::onMeasure(const GuiLayoutContext&, const GuiSize&)
{
    return GuiSizeZero;
}

void ego::gui::GuiWidget::onArrange(const GuiLayoutContext&, const GuiRect&) {}

void ego::gui::GuiWidget::onPaint(GuiPaintContext&) const {}
