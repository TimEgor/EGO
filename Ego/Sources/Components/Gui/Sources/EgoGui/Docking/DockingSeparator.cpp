#include "DockingSeparator.h"

#include <algorithm>

#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Docking/DockingSplit.h"
#include "EgoGui/Input/WidgetUpdateContext.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::DockingSeparatorPointer ego::gui::DockingSeparator::Create(DockingAxis _axis)
{
    switch (_axis)
    {
    case DockingAxis::Horizontal:
    case DockingAxis::Vertical:
        break;

    default:
        return nullptr;
    }

    return new DockingSeparator(_axis);
}

ego::gui::DockingSeparator::DockingSeparator(DockingAxis _axis)
    : m_axis(_axis)
{
}

void ego::gui::DockingSeparator::clearInteraction()
{
    m_isHovered = false;
    m_isPressed = false;
}

ego::gui::DockingSplitPointer ego::gui::DockingSeparator::getSplit() const
{
    const WidgetPointer parent = getParent();

    return parent && rtti::IsObjectBasedOn<DockingSplit>(*parent) ? ego::StaticPointerCast<DockingSplit>(parent) : nullptr;
}

ego::gui::InputReply ego::gui::DockingSeparator::onPointerMove(WidgetUpdateContext&, const PointerMoveEvent& _event)
{
    m_isHovered = getLayoutBounds().contains(_event.m_position);
    if (!m_isPressed)
    {
        return InputReply::Unhandled;
    }

    const DockingSplitPointer split = getSplit();
    if (split)
    {
        split->updateRatio(_event.m_position, m_minimumSpaceSize, m_separatorThickness);
    }

    return InputReply::Handled;
}

ego::gui::InputReply ego::gui::DockingSeparator::onMouseButton(WidgetUpdateContext&, const MouseButtonEvent& _event)
{
    if (_event.m_key != MouseInputKey::ButtonLeft)
    {
        return InputReply::Unhandled;
    }

    if (_event.m_action == InputButtonAction::Pressed && getLayoutBounds().contains(_event.m_position) && !m_isPressed)
    {
        m_isHovered = true;
        m_isPressed = true;

        return InputReply::Capture;
    }

    if (_event.m_action == InputButtonAction::Released && m_isPressed)
    {
        const DockingSplitPointer split = getSplit();
        m_isPressed = false;
        m_isHovered = getLayoutBounds().contains(_event.m_position);
        if (split)
        {
            split->updateRatio(_event.m_position, m_minimumSpaceSize, m_separatorThickness);
        }

        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::DockingSeparator::onPointerEnter(WidgetUpdateContext&, const Position& _position, const InputModifiers&)
{
    m_isHovered = getLayoutBounds().contains(_position);
}

void ego::gui::DockingSeparator::onPointerLeave(WidgetUpdateContext&, const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::DockingSeparator::onPointerCaptureLost(WidgetUpdateContext&, const Position&)
{
    clearInteraction();
}

ego::gui::Size ego::gui::DockingSeparator::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const DockingStyle& style = _context.getTheme().m_docking;
    m_minimumSpaceSize = Size((std::max)(0.0f, style.m_minimumSpaceSize.m_x), (std::max)(0.0f, style.m_minimumSpaceSize.m_y));
    m_separatorThickness = (std::max)(0.0f, style.m_separatorThickness);

    return m_axis == DockingAxis::Horizontal ? Size(m_separatorThickness, _constraints.m_maximumSize.m_y) :
                                               Size(_constraints.m_maximumSize.m_x, m_separatorThickness);
}

void ego::gui::DockingSeparator::drawBaseLayer(PaintContext& _context) const
{
    _context.drawBox(getLayoutBounds(), _context.getTheme().m_docking.m_separator.resolve(m_isHovered, m_isPressed));
}
