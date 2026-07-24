#include "DockingSplit.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/DockingSeparator.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::DockingSplitPointer ego::gui::DockingSplit::Create(DockingAxis _axis, float _ratio)
{
    switch (_axis)
    {
    case DockingAxis::Horizontal:
    case DockingAxis::Vertical:
        break;

    default:
        return nullptr;
    }

    if (!std::isfinite(_ratio))
    {
        return nullptr;
    }

    const DockingSplitPointer split = new DockingSplit(_axis, (std::clamp)(_ratio, 0.0f, 1.0f));

    return split->initialize() ? split : nullptr;
}

ego::gui::DockingSplit::DockingSplit(DockingAxis _axis, float _ratio)
    : m_axis(_axis),
      m_ratio(_ratio)
{
}

bool ego::gui::DockingSplit::initialize()
{
    m_separator = DockingSeparator::Create(m_axis);

    return m_separator && attachChild(m_separator);
}

ego::gui::DockingAxis ego::gui::DockingSplit::getAxis() const
{
    return m_axis;
}

void ego::gui::DockingSplit::setRatio(float _ratio)
{
    if (!std::isfinite(_ratio))
    {
        return;
    }

    const float ratio = (std::clamp)(_ratio, 0.0f, 1.0f);
    if (m_ratio == ratio)
    {
        return;
    }

    m_ratio = ratio;
    invalidateLayout();
}

ego::gui::DockingNodePointer ego::gui::DockingSplit::getFirst() const
{
    return m_first;
}

ego::gui::DockingNodePointer ego::gui::DockingSplit::getSecond() const
{
    return m_second;
}

bool ego::gui::DockingSplit::setChildren(const DockingNodePointer& _first, const DockingNodePointer& _second)
{
    if (m_first || m_second || !_first || !_second || !attachChild(_first))
    {
        return false;
    }

    if (!attachChild(_second))
    {
        detachChild(_first);

        return false;
    }

    m_first = _first;
    m_second = _second;
    notifyTreeChanged();

    return true;
}

bool ego::gui::DockingSplit::replaceChildNode(const DockingNodePointer& _node, const DockingNodePointer& _replacement)
{
    if (!_node || !_replacement)
    {
        return false;
    }

    bool replaceFirst = false;
    if (m_first.get() == _node.get())
    {
        replaceFirst = true;
    }
    else if (m_second.get() != _node.get())
    {
        return false;
    }

    const DockingNodePointer previousChild = replaceFirst ? m_first : m_second;
    if (!detachChild(previousChild))
    {
        return false;
    }

    if (!attachChild(_replacement))
    {
        const bool restored = attachChild(previousChild);
        EGO_ASSERT(restored);

        return false;
    }

    if (replaceFirst)
    {
        m_first = _replacement;
    }
    else
    {
        m_second = _replacement;
    }
    notifyTreeChanged();

    return true;
}

ego::gui::DockingNodePointer ego::gui::DockingSplit::releaseSibling(const DockingNodePointer& _node)
{
    if (!_node || !m_first || !m_second)
    {
        return nullptr;
    }

    DockingNodePointer sibling;
    if (m_first.get() == _node.get())
    {
        sibling = m_second;
    }
    else if (m_second.get() == _node.get())
    {
        sibling = m_first;
    }
    else
    {
        return nullptr;
    }

    if (!m_first->isDirectChildOf(*this) || !m_second->isDirectChildOf(*this))
    {
        EGO_ASSERT_FAIL_MESSAGE("Docking split children must be detachable.");

        return nullptr;
    }

    const bool firstDetached = detachChild(m_first);
    EGO_ASSERT(firstDetached);
    if (!detachChild(m_second))
    {
        const bool restored = attachChild(m_first);
        EGO_ASSERT(restored);

        return nullptr;
    }

    m_first = nullptr;
    m_second = nullptr;
    notifyTreeChanged();

    return sibling;
}

void ego::gui::DockingSplit::updateRatio(const Position& _position, const Size& _minimumSpaceSize, float _separatorThickness)
{
    if (!m_first || !m_second || !m_separator || m_first->getSpaceCount() == 0 || m_second->getSpaceCount() == 0)
    {
        return;
    }

    const Rect& splitBounds = getLayoutBounds();
    const Rect& separatorBounds = m_separator->getLayoutBounds();
    const bool horizontal = m_axis == DockingAxis::Horizontal;
    const float totalExtent = horizontal ? splitBounds.m_size.m_x : splitBounds.m_size.m_y;
    const float separatorExtent = horizontal ? separatorBounds.m_size.m_x : separatorBounds.m_size.m_y;
    const float availableExtent = (std::max)(0.0f, totalExtent - separatorExtent);
    if (availableExtent <= 0.0f)
    {
        return;
    }

    const Size firstMinimumSize = m_first->getMinimumSize(_minimumSpaceSize, _separatorThickness);
    const Size secondMinimumSize = m_second->getMinimumSize(_minimumSpaceSize, _separatorThickness);
    const float firstMinimumExtent = horizontal ? firstMinimumSize.m_x : firstMinimumSize.m_y;
    const float secondMinimumExtent = horizontal ? secondMinimumSize.m_x : secondMinimumSize.m_y;
    float minimumRatio = firstMinimumExtent / availableExtent;
    float maximumRatio = 1.0f - secondMinimumExtent / availableExtent;
    if (minimumRatio > maximumRatio)
    {
        const float totalMinimumExtent = firstMinimumExtent + secondMinimumExtent;
        minimumRatio = totalMinimumExtent > 0.0f ? firstMinimumExtent / totalMinimumExtent : 0.5f;
        maximumRatio = minimumRatio;
    }

    const float pointerOffset = horizontal ? _position.m_x - splitBounds.m_position.m_x : _position.m_y - splitBounds.m_position.m_y;
    setRatio((std::clamp)((pointerOffset - separatorExtent * 0.5f) / availableExtent, minimumRatio, maximumRatio));
}

size_t ego::gui::DockingSplit::getSpaceCount() const
{
    return (m_first ? m_first->getSpaceCount() : 0) + (m_second ? m_second->getSpaceCount() : 0);
}

size_t ego::gui::DockingSplit::getSpaceCountExcluding(const DockingSpace& _space) const
{
    return (m_first ? m_first->getSpaceCountExcluding(_space) : 0) + (m_second ? m_second->getSpaceCountExcluding(_space) : 0);
}

ego::gui::Size ego::gui::DockingSplit::getMinimumSize(const Size& _minimumSpaceSize, float _separatorThickness) const
{
    const Size firstMinimumSize = m_first ? m_first->getMinimumSize(_minimumSpaceSize, _separatorThickness) : SizeZero;
    const Size secondMinimumSize = m_second ? m_second->getMinimumSize(_minimumSpaceSize, _separatorThickness) : SizeZero;
    const size_t firstSpaceCount = m_first ? m_first->getSpaceCount() : 0;
    const size_t secondSpaceCount = m_second ? m_second->getSpaceCount() : 0;
    if (firstSpaceCount == 0)
    {
        return secondMinimumSize;
    }

    if (secondSpaceCount == 0)
    {
        return firstMinimumSize;
    }

    const float separatorThickness = (std::max)(0.0f, _separatorThickness);
    if (m_axis == DockingAxis::Horizontal)
    {
        return Size(firstMinimumSize.m_x + separatorThickness + secondMinimumSize.m_x, (std::max)(firstMinimumSize.m_y, secondMinimumSize.m_y));
    }

    return Size((std::max)(firstMinimumSize.m_x, secondMinimumSize.m_x), firstMinimumSize.m_y + separatorThickness + secondMinimumSize.m_y);
}

ego::gui::Size ego::gui::DockingSplit::getMinimumSizeExcluding(const DockingSpace& _space, const Size& _minimumSpaceSize, float _separatorThickness) const
{
    const Size firstMinimumSize = m_first ? m_first->getMinimumSizeExcluding(_space, _minimumSpaceSize, _separatorThickness) : SizeZero;
    const Size secondMinimumSize = m_second ? m_second->getMinimumSizeExcluding(_space, _minimumSpaceSize, _separatorThickness) : SizeZero;
    const size_t firstSpaceCount = m_first ? m_first->getSpaceCountExcluding(_space) : 0;
    const size_t secondSpaceCount = m_second ? m_second->getSpaceCountExcluding(_space) : 0;
    if (firstSpaceCount == 0)
    {
        return secondMinimumSize;
    }

    if (secondSpaceCount == 0)
    {
        return firstMinimumSize;
    }

    const float separatorThickness = (std::max)(0.0f, _separatorThickness);
    if (m_axis == DockingAxis::Horizontal)
    {
        return Size(firstMinimumSize.m_x + separatorThickness + secondMinimumSize.m_x, (std::max)(firstMinimumSize.m_y, secondMinimumSize.m_y));
    }

    return Size((std::max)(firstMinimumSize.m_x, secondMinimumSize.m_x), firstMinimumSize.m_y + separatorThickness + secondMinimumSize.m_y);
}

void ego::gui::DockingSplit::clearInteraction()
{
    if (m_separator)
    {
        m_separator->clearInteraction();
    }

    if (m_first)
    {
        m_first->clearInteraction();
    }

    if (m_second)
    {
        m_second->clearInteraction();
    }
}

void ego::gui::DockingSplit::calculateChildBounds(
    const Rect& _bounds,
    const DockingStyle& _style,
    Rect& _firstBounds,
    Rect& _separatorBounds,
    Rect& _secondBounds) const
{
    const bool horizontal = m_axis == DockingAxis::Horizontal;
    const float totalExtent = horizontal ? _bounds.m_size.m_x : _bounds.m_size.m_y;
    const size_t firstSpaceCount = m_first ? m_first->getSpaceCount() : 0;
    const size_t secondSpaceCount = m_second ? m_second->getSpaceCount() : 0;
    const bool hasBothChildren = firstSpaceCount > 0 && secondSpaceCount > 0;
    const float separatorThickness = hasBothChildren ? (std::min)((std::max)(0.0f, _style.m_separatorThickness), totalExtent) : 0.0f;
    const float availableExtent = (std::max)(0.0f, totalExtent - separatorThickness);
    float ratio = (std::clamp)(m_ratio, 0.0f, 1.0f);
    if (hasBothChildren && availableExtent > 0.0f)
    {
        const Size firstMinimumSize = m_first->getMinimumSize(_style.m_minimumSpaceSize, _style.m_separatorThickness);
        const Size secondMinimumSize = m_second->getMinimumSize(_style.m_minimumSpaceSize, _style.m_separatorThickness);
        const float firstMinimumExtent = horizontal ? firstMinimumSize.m_x : firstMinimumSize.m_y;
        const float secondMinimumExtent = horizontal ? secondMinimumSize.m_x : secondMinimumSize.m_y;
        float minimumRatio = firstMinimumExtent / availableExtent;
        float maximumRatio = 1.0f - secondMinimumExtent / availableExtent;
        if (minimumRatio > maximumRatio)
        {
            const float totalMinimumExtent = firstMinimumExtent + secondMinimumExtent;
            minimumRatio = totalMinimumExtent > 0.0f ? firstMinimumExtent / totalMinimumExtent : 0.5f;
            maximumRatio = minimumRatio;
        }

        ratio = (std::clamp)(ratio, minimumRatio, maximumRatio);
    }
    else if (firstSpaceCount == 0)
    {
        ratio = 0.0f;
    }
    else if (secondSpaceCount == 0)
    {
        ratio = 1.0f;
    }

    const float firstExtent = std::floor(availableExtent * ratio);
    const float secondExtent = firstSpaceCount == 0 && secondSpaceCount == 0 ? 0.0f : availableExtent - firstExtent;

    _firstBounds = _bounds;
    _secondBounds = _bounds;
    if (horizontal)
    {
        _firstBounds.m_size.m_x = firstExtent;
        _secondBounds.m_position.m_x += firstExtent + separatorThickness;
        _secondBounds.m_size.m_x = secondExtent;
        _separatorBounds = Rect(_bounds.m_position.m_x + firstExtent, _bounds.m_position.m_y, separatorThickness, _bounds.m_size.m_y);
    }
    else
    {
        _firstBounds.m_size.m_y = firstExtent;
        _secondBounds.m_position.m_y += firstExtent + separatorThickness;
        _secondBounds.m_size.m_y = secondExtent;
        _separatorBounds = Rect(_bounds.m_position.m_x, _bounds.m_position.m_y + firstExtent, _bounds.m_size.m_x, separatorThickness);
    }
}

ego::gui::Size ego::gui::DockingSplit::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    Rect firstBounds;
    Rect separatorBounds;
    Rect secondBounds;
    calculateChildBounds(Rect(PositionZero, _constraints.m_maximumSize), _context.getTheme().m_docking, firstBounds, separatorBounds, secondBounds);
    if (m_first)
    {
        m_first->updatePreferredSize(_context, LayoutConstraints(firstBounds.m_size));
    }

    if (m_separator)
    {
        m_separator->updatePreferredSize(_context, LayoutConstraints(separatorBounds.m_size));
    }

    if (m_second)
    {
        m_second->updatePreferredSize(_context, LayoutConstraints(secondBounds.m_size));
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingSplit::updateGeometry(const LayoutContext& _context)
{
    Rect firstBounds;
    Rect separatorBounds;
    Rect secondBounds;
    calculateChildBounds(getLayoutBounds(), _context.getTheme().m_docking, firstBounds, separatorBounds, secondBounds);
    if (m_first)
    {
        m_first->applyLayout(_context, firstBounds);
    }

    if (m_separator)
    {
        m_separator->applyLayout(_context, separatorBounds);
    }

    if (m_second)
    {
        m_second->applyLayout(_context, secondBounds);
    }
}

size_t ego::gui::DockingSplit::getChildCount() const
{
    return m_first && m_second ? 3 : (m_separator ? 1 : 0);
}

ego::gui::WidgetPointer ego::gui::DockingSplit::getChild(size_t _index) const
{
    if (!m_first || !m_second)
    {
        return _index == 0 ? m_separator : nullptr;
    }

    switch (_index)
    {
    case 0:
        return m_first;

    case 1:
        return m_separator;

    case 2:
        return m_second;

    default:
        return nullptr;
    }
}
