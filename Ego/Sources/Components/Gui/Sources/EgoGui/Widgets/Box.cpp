#include "Box.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Theme/Theme.h"

ego::gui::BoxSlot ego::gui::BoxSlot::Content(const Margin& _padding, BoxCrossAlignment _alignment)
{
    BoxSlot slot;
    slot.m_padding = _padding;
    slot.m_alignment = _alignment;
    return slot;
}

ego::gui::BoxSlot ego::gui::BoxSlot::Fill(float _weight, const Margin& _padding, BoxCrossAlignment _alignment)
{
    BoxSlot slot;
    slot.m_padding = _padding;
    slot.m_alignment = _alignment;
    slot.m_fillWeight = (std::max)(0.0f, _weight);
    return slot;
}

const ego::gui::Margin& ego::gui::BoxSlot::getPadding() const
{
    return m_padding;
}

ego::gui::BoxCrossAlignment ego::gui::BoxSlot::getAlignment() const
{
    return m_alignment;
}

float ego::gui::BoxSlot::getFillWeight() const
{
    return m_fillWeight;
}

ego::gui::BoxChild::BoxChild(const WidgetPointer& _widget)
    : m_widget(_widget)
{
}

ego::gui::BoxChild::BoxChild(const WidgetPointer& _widget, const BoxSlot& _slot)
    : m_widget(_widget),
      m_slot(_slot)
{
}

ego::gui::Box::Box(Orientation _orientation, bool _useContentPadding)
    : m_orientation(_orientation),
      m_useContentPadding(_useContentPadding)
{
}

bool ego::gui::Box::addChild(const WidgetPointer& _widget, const BoxSlot& _slot)
{
    if (!attachChild(_widget))
    {
        return false;
    }

    m_children.push_back({_widget, _slot});
    notifyTreeChanged();
    return true;
}

ego::gui::WidgetPointer ego::gui::Box::removeChild(const WidgetPointer& _widget)
{
    if (!_widget)
    {
        return nullptr;
    }

    const ChildCollection::iterator childIt = std::find_if(
        m_children.begin(),
        m_children.end(),
        [&_widget](const BoxChild& _child)
        {
            return _child.m_widget.get() == _widget.get();
        });
    if (childIt == m_children.end())
    {
        return nullptr;
    }

    const BoxChild removedChild = *childIt;
    m_children.erase(childIt);
    if (!detachChild(removedChild.m_widget))
    {
        m_children.push_back(removedChild);
        return nullptr;
    }
    notifyTreeChanged();
    return removedChild.m_widget;
}

void ego::gui::Box::clearChildren()
{
    if (m_children.empty())
    {
        return;
    }

    ChildCollection children;
    children.swap(m_children);
    for (const BoxChild& child : children)
    {
        const bool detached = detachChild(child.m_widget);
        EGO_ASSERT(detached);
    }
    notifyTreeChanged();
}

ego::gui::Size ego::gui::Box::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const Size& maximumSize = _constraints.m_maximumSize;
    const LayoutStyle& layout = _context.getTheme().m_layout;
    const bool isVertical = m_orientation == Orientation::Vertical;
    const Margin contentPadding = m_useContentPadding ? layout.m_contentPadding : Margin();
    const float containerMainPadding = isVertical ? contentPadding.getVertical() : contentPadding.getHorizontal();
    const float containerCrossPadding = isVertical ? contentPadding.getHorizontal() : contentPadding.getVertical();
    const float availableMainExtent = (std::max)(0.0f, (isVertical ? maximumSize.m_y : maximumSize.m_x) - containerMainPadding);
    const float availableCrossExtent = (std::max)(0.0f, (isVertical ? maximumSize.m_x : maximumSize.m_y) - containerCrossPadding);
    const float spacing = (std::max)(0.0f, layout.m_spacing);

    float desiredMainExtent = containerMainPadding;
    float desiredCrossExtent = containerCrossPadding;
    size_t visibleChildCount = 0;
    for (const BoxChild& child : m_children)
    {
        if (child.m_widget->isCollapsed())
        {
            child.m_widget->updatePreferredSize(_context, LayoutConstraints(SizeZero));
            continue;
        }

        const Margin& padding = child.m_slot.getPadding();
        const float mainPadding = isVertical ? padding.getVertical() : padding.getHorizontal();
        const float crossPadding = isVertical ? padding.getHorizontal() : padding.getVertical();
        const float childMainExtent = (std::max)(0.0f, availableMainExtent - mainPadding);
        const float childCrossExtent = (std::max)(0.0f, availableCrossExtent - crossPadding);
        const Size childAvailableSize = isVertical ? Size(childCrossExtent, childMainExtent) : Size(childMainExtent, childCrossExtent);
        const Size childPreferredSize = child.m_widget->updatePreferredSize(_context, LayoutConstraints(childAvailableSize));

        if (visibleChildCount > 0)
        {
            desiredMainExtent += spacing;
        }
        desiredMainExtent += (isVertical ? childPreferredSize.m_y : childPreferredSize.m_x) + mainPadding;
        desiredCrossExtent =
            (std::max)(desiredCrossExtent, containerCrossPadding + (isVertical ? childPreferredSize.m_x : childPreferredSize.m_y) + crossPadding);
        ++visibleChildCount;
    }

    return isVertical ? Size(desiredCrossExtent, desiredMainExtent) : Size(desiredMainExtent, desiredCrossExtent);
}

void ego::gui::Box::updateGeometry(const LayoutContext& _context)
{
    const Rect& layoutBounds = getLayoutBounds();
    const LayoutStyle& layout = _context.getTheme().m_layout;
    const bool isVertical = m_orientation == Orientation::Vertical;
    const Margin contentPadding = m_useContentPadding ? layout.m_contentPadding : Margin();
    const float containerLeadingMainPadding = isVertical ? contentPadding.m_top : contentPadding.m_left;
    const float containerTrailingMainPadding = isVertical ? contentPadding.m_bottom : contentPadding.m_right;
    const float containerLeadingCrossPadding = isVertical ? contentPadding.m_left : contentPadding.m_top;
    const float containerTrailingCrossPadding = isVertical ? contentPadding.m_right : contentPadding.m_bottom;
    const float mainStart = (isVertical ? layoutBounds.m_position.m_y : layoutBounds.m_position.m_x) + containerLeadingMainPadding;
    const float crossStart = (isVertical ? layoutBounds.m_position.m_x : layoutBounds.m_position.m_y) + containerLeadingCrossPadding;
    const float mainExtent =
        (std::max)(0.0f, (isVertical ? layoutBounds.m_size.m_y : layoutBounds.m_size.m_x) - containerLeadingMainPadding - containerTrailingMainPadding);
    const float crossExtent =
        (std::max)(0.0f, (isVertical ? layoutBounds.m_size.m_x : layoutBounds.m_size.m_y) - containerLeadingCrossPadding - containerTrailingCrossPadding);
    const float spacing = (std::max)(0.0f, layout.m_spacing);

    float fixedMainExtent = 0.0f;
    float totalFillWeight = 0.0f;
    size_t visibleChildCount = 0;
    for (const BoxChild& child : m_children)
    {
        if (child.m_widget->isCollapsed())
        {
            child.m_widget->applyLayout(_context, Rect(layoutBounds.m_position, SizeZero));
            continue;
        }

        if (visibleChildCount > 0)
        {
            fixedMainExtent += spacing;
        }
        const Margin& padding = child.m_slot.getPadding();
        const float fillWeight = child.m_slot.getFillWeight();
        fixedMainExtent += isVertical ? padding.getVertical() : padding.getHorizontal();
        if (fillWeight > 0.0f)
        {
            totalFillWeight += fillWeight;
        }
        else
        {
            const Size& preferredSize = child.m_widget->getPreferredSize();
            fixedMainExtent += isVertical ? preferredSize.m_y : preferredSize.m_x;
        }
        ++visibleChildCount;
    }

    const float fillExtent = (std::max)(0.0f, mainExtent - fixedMainExtent);
    float currentMainPosition = mainStart;
    size_t arrangedChildCount = 0;
    for (const BoxChild& child : m_children)
    {
        if (child.m_widget->isCollapsed())
        {
            continue;
        }

        if (arrangedChildCount > 0)
        {
            currentMainPosition += spacing;
        }
        const Size& preferredSize = child.m_widget->getPreferredSize();
        const Margin& padding = child.m_slot.getPadding();
        const float fillWeight = child.m_slot.getFillWeight();
        const BoxCrossAlignment alignment = child.m_slot.getAlignment();
        const float leadingMainPadding = isVertical ? padding.m_top : padding.m_left;
        const float trailingMainPadding = isVertical ? padding.m_bottom : padding.m_right;
        const float leadingCrossPadding = isVertical ? padding.m_left : padding.m_top;
        const float crossPadding = isVertical ? padding.getHorizontal() : padding.getVertical();

        float childMainExtent = isVertical ? preferredSize.m_y : preferredSize.m_x;
        if (fillWeight > 0.0f && totalFillWeight > 0.0f)
        {
            childMainExtent = fillExtent * fillWeight / totalFillWeight;
        }

        const float availableCrossExtent = (std::max)(0.0f, crossExtent - crossPadding);
        float childCrossExtent = (std::min)(isVertical ? preferredSize.m_x : preferredSize.m_y, availableCrossExtent);
        float childCrossPosition = crossStart + leadingCrossPadding;
        if (alignment == BoxCrossAlignment::Stretch)
        {
            childCrossExtent = availableCrossExtent;
        }
        else if (alignment == BoxCrossAlignment::Center)
        {
            childCrossPosition += (availableCrossExtent - childCrossExtent) * 0.5f;
        }
        else if (alignment == BoxCrossAlignment::End)
        {
            childCrossPosition += availableCrossExtent - childCrossExtent;
        }

        const float childMainPosition = currentMainPosition + leadingMainPadding;
        const Rect childRect = isVertical ? Rect(childCrossPosition, childMainPosition, childCrossExtent, childMainExtent) :
                                            Rect(childMainPosition, childCrossPosition, childMainExtent, childCrossExtent);
        child.m_widget->applyLayout(_context, childRect);
        currentMainPosition += leadingMainPadding + childMainExtent + trailingMainPadding;
        ++arrangedChildCount;
    }
}

size_t ego::gui::Box::getChildCount() const
{
    return m_children.size();
}

ego::gui::WidgetPointer ego::gui::Box::getChild(size_t _index) const
{
    return m_children[_index].m_widget;
}

ego::gui::BoxChild ego::gui::Fill(const WidgetPointer& _widget, float _weight)
{
    return BoxChild(_widget, BoxSlot::Fill(_weight));
}
