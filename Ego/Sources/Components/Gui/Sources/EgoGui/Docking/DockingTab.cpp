#include "DockingTab.h"

#include <algorithm>

#include "EgoGui/Docking/DockingSpace.h"
#include "EgoGui/Input/Input.h"
#include "EgoGui/Input/InputContext.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Window.h"

namespace
{
    using WindowAccessor = ego::gui::Window::HierarchyAccessor;
} // namespace

bool ego::gui::DockingTab::HierarchyAccessor::AttachToSpace(DockingTab& _tab, const DockingSpacePointer& _space)
{
    if (!_space || _tab.m_space.lock())
    {
        return false;
    }

    _tab.m_space = _space;

    return true;
}

bool ego::gui::DockingTab::HierarchyAccessor::IsAttachedToSpace(const DockingTab& _tab, const DockingSpace& _space)
{
    const DockingSpacePointer space = _tab.m_space.lock();

    return space.get() == &_space;
}

void ego::gui::DockingTab::HierarchyAccessor::DetachFromSpace(DockingTab& _tab)
{
    _tab.m_space.reset();
}

ego::gui::DockingTab::~DockingTab()
{
    if (m_window)
    {
        WindowAccessor::SetDocked(*m_window, false);
    }
}

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
    WindowAccessor::SetDocked(*m_window, true);

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
    WindowAccessor::SetDocked(*window, false);
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
    return m_space.lock();
}

ego::gui::InputReply ego::gui::DockingTab::onPointerMove(InputContext& _context, const PointerMoveEvent& _event)
{
    m_isHovered = m_headerBounds.contains(_event.m_position);
    if (!m_isPressed || !m_window)
    {
        return InputReply::Unhandled;
    }

    _context.updateWindowDrag(m_window, _event.m_position);

    return InputReply::Handled;
}

ego::gui::InputReply ego::gui::DockingTab::onMouseButton(InputContext& _context, const MouseButtonEvent& _event)
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
        _context.beginDockedWindowDrag(m_window, _event.m_position);

        return InputReply::Capture;
    }

    if (_event.m_action == InputButtonAction::Released && m_isPressed)
    {
        m_isPressed = false;
        m_isHovered = m_headerBounds.contains(_event.m_position);
        if (m_window)
        {
            _context.finishWindowDrag(m_window, _event.m_position);
        }

        return InputReply::Handled;
    }

    return InputReply::Unhandled;
}

void ego::gui::DockingTab::onPointerEnter(const Position& _position, const InputModifiers&)
{
    m_isHovered = m_headerBounds.contains(_position);
}

void ego::gui::DockingTab::onPointerLeave(const Position&, const InputModifiers&)
{
    m_isHovered = false;
}

void ego::gui::DockingTab::onPointerCaptureLost(const Position&)
{
    clearInteraction();
}

ego::gui::Size ego::gui::DockingTab::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    if (m_window)
    {
        _context.measure(*m_window, LayoutConstraints(m_contentBounds.m_size));
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingTab::updateGeometry(const LayoutContext& _context)
{
    if (m_window)
    {
        _context.arrange(*m_window, m_contentBounds);
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
