#include "DockingTab.h"

#include <algorithm>

#include "EgoCore/RTTI/RTTI.h"

#include "EgoGui/Docking/DockingOverlay.h"
#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Input/WidgetUpdateContext.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Window.h"

ego::gui::DockingTabPointer ego::gui::DockingTab::Create(const WindowPointer& _window)
{
    const DockingTabPointer tab = new DockingTab();

    return tab->initialize(_window) ? tab : nullptr;
}

bool ego::gui::DockingTab::initialize(const WindowPointer& _window)
{
    if (!_window || !attachChild(_window))
    {
        return false;
    }

    m_window = _window;

    return true;
}

ego::gui::WindowPointer ego::gui::DockingTab::getWindow() const
{
    return m_window;
}

ego::gui::WindowPointer ego::gui::DockingTab::releaseWindow()
{
    const WindowPointer window = m_window;
    if (!window || !detachChild(window))
    {
        return nullptr;
    }

    m_window = nullptr;
    notifyTreeChanged();

    return window;
}

bool ego::gui::DockingTab::isSelected() const
{
    const DockingSpacePointer space = getSpace();

    return space && space->isTabSelected(*this);
}

void ego::gui::DockingTab::setArrangement(const Rect& _headerBounds, const Rect& _contentBounds)
{
    m_headerBounds = _headerBounds;
    m_contentBounds = _contentBounds;
}

void ego::gui::DockingTab::clearInteraction()
{
    m_isHovered = false;
    m_isPressed = false;
}

bool ego::gui::DockingTab::hitTest(const Position& _position) const
{
    return isVisible() && (m_headerBounds.contains(_position) || (isSelected() && m_contentBounds.contains(_position)));
}

ego::gui::DockingSpacePointer ego::gui::DockingTab::getSpace() const
{
    const WidgetPointer parent = getParent();

    return parent && rtti::IsObjectBasedOn<DockingSpace>(*parent) ? ego::StaticPointerCast<DockingSpace>(parent) : nullptr;
}

ego::gui::InputReply ego::gui::DockingTab::onPointerMove(WidgetUpdateContext& _context, const PointerMoveEvent& _event)
{
    m_isHovered = m_headerBounds.contains(_event.m_position);
    if (!m_isPressed || !m_window)
    {
        return InputReply::Unhandled;
    }

    const DockingOverlayPointer dockingOverlay = _context.getDockingOverlay();
    if (dockingOverlay)
    {
        dockingOverlay->updateDrag(_context, m_window, _event.m_position);
    }

    return InputReply::Handled;
}

ego::gui::InputReply ego::gui::DockingTab::onMouseButton(WidgetUpdateContext& _context, const MouseButtonEvent& _event)
{
    if (_event.m_key != MouseInputKey::ButtonLeft)
    {
        return InputReply::Unhandled;
    }

    if (_event.m_action == InputButtonAction::Pressed && m_headerBounds.contains(_event.m_position) && !m_isPressed)
    {
        const DockingSpacePointer space = getSpace();
        if (!space || !space->selectTab(*this) || !m_window)
        {
            return InputReply::Unhandled;
        }

        m_isHovered = true;
        m_isPressed = true;
        const DockingOverlayPointer dockingOverlay = _context.getDockingOverlay();
        if (dockingOverlay)
        {
            dockingOverlay->beginDockedDrag(_context, m_window, _event.m_position);
        }

        return InputReply::Capture;
    }

    if (_event.m_action == InputButtonAction::Released && m_isPressed)
    {
        m_isPressed = false;
        m_isHovered = m_headerBounds.contains(_event.m_position);
        if (m_window)
        {
            const DockingOverlayPointer dockingOverlay = _context.getDockingOverlay();
            if (dockingOverlay)
            {
                dockingOverlay->finishDrag(_context, m_window, _event.m_position);
            }
        }

        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::DockingTab::onPointerEnter(WidgetUpdateContext&, const Position& _position, const InputModifiers&)
{
    m_isHovered = m_headerBounds.contains(_position);
}

void ego::gui::DockingTab::onPointerLeave(WidgetUpdateContext&, const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::DockingTab::onPointerCaptureLost(WidgetUpdateContext& _context, const Position&)
{
    const bool wasPressed = m_isPressed;
    clearInteraction();
    if (wasPressed && m_window)
    {
        const DockingOverlayPointer dockingOverlay = _context.getDockingOverlay();
        if (dockingOverlay)
        {
            dockingOverlay->cancelDrag(m_window);
        }
    }
}

ego::gui::Size ego::gui::DockingTab::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    if (m_window)
    {
        m_window->updatePreferredSize(_context, LayoutConstraints(m_contentBounds.m_size));
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingTab::updateGeometry(const LayoutContext& _context)
{
    if (m_window)
    {
        m_window->applyLayout(_context, m_contentBounds);
    }
}

void ego::gui::DockingTab::drawBaseLayer(PaintContext& _context) const
{
    if (!m_window || m_headerBounds.m_size.m_x <= 0.0f || m_headerBounds.m_size.m_y <= 0.0f)
    {
        return;
    }

    const DockingStyle& style = _context.getTheme().m_docking;
    const NormalizedColorRGBA& tabColor = isSelected() ? style.m_tabActive : style.m_tab.resolve(m_isHovered, m_isPressed);
    _context.drawBox(m_headerBounds, tabColor);

    const Rect textBounds(
        m_headerBounds.m_position.m_x + style.m_tabPadding.m_left,
        m_headerBounds.m_position.m_y + style.m_tabPadding.m_top,
        (std::max)(0.0f, m_headerBounds.m_size.m_x - style.m_tabPadding.getHorizontal()),
        (std::max)(0.0f, m_headerBounds.m_size.m_y - style.m_tabPadding.getVertical()));
    _context.drawText(m_window->getTitle(), textBounds, _context.getTheme().m_typography.m_primary);
}

bool ego::gui::DockingTab::isChildHitTestVisible(const Position& _position) const
{
    return isSelected() && m_contentBounds.contains(_position);
}

size_t ego::gui::DockingTab::getChildCount() const
{
    return m_window ? 1 : 0;
}

ego::gui::WidgetPointer ego::gui::DockingTab::getChild(size_t) const
{
    return m_window;
}

bool ego::gui::DockingTab::isChildActive(size_t) const
{
    return isSelected();
}
