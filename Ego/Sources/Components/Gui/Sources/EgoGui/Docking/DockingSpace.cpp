#include "DockingSpace.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/DockingTab.h"
#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Window.h"

namespace
{
    using TabAccessor = ego::gui::DockingTab::HierarchyAccessor;
} // namespace

ego::gui::DockingSpacePointer ego::gui::DockingSpace::Create(DockingSpaceID _id)
{
    return _id != InvalidDockingSpaceID ? DockingSpacePointer(new DockingSpace(_id)) : nullptr;
}

ego::gui::DockingSpace::DockingSpace(DockingSpaceID _id)
    : m_id(_id)
{
}

ego::gui::DockingSpaceID ego::gui::DockingSpace::getID() const
{
    return m_id;
}

size_t ego::gui::DockingSpace::getWindowCount() const
{
    return m_tabs.size();
}

ego::gui::WindowPointer ego::gui::DockingSpace::getWindow(size_t _index) const
{
    const DockingTabPointer tab = _index < m_tabs.size() ? m_tabs[_index] : nullptr;

    return tab ? tab->getWindow() : nullptr;
}

ego::gui::DockingSpace::WindowCollection ego::gui::DockingSpace::getWindows() const
{
    WindowCollection windows;
    windows.reserve(m_tabs.size());
    for (const DockingTabPointer& tab : m_tabs)
    {
        if (tab && tab->getWindow())
        {
            windows.push_back(tab->getWindow());
        }
    }

    return windows;
}

ego::gui::DockingTabPointer ego::gui::DockingSpace::findTab(const WindowPointer& _window) const
{
    const TabCollection::const_iterator tabIt = std::find_if(
        m_tabs.begin(),
        m_tabs.end(),
        [&_window](const DockingTabPointer& _tab)
        {
            return _tab && _tab->getWindow().get() == _window.get();
        });

    return tabIt != m_tabs.end() ? *tabIt : nullptr;
}

bool ego::gui::DockingSpace::addWindow(const WindowPointer& _window)
{
    if (!_window || findTab(_window))
    {
        return false;
    }

    const DockingTabPointer tab = DockingTab::Create(_window);
    if (!tab)
    {
        return false;
    }

    return insertTab(tab);
}

ego::gui::WindowPointer ego::gui::DockingSpace::removeWindow(const WindowPointer& _window)
{
    const DockingTabPointer tab = releaseTab(_window);
    if (!tab)
    {
        return nullptr;
    }

    const WindowPointer window = tab->releaseWindow();
    EGO_ASSERT(window);

    return window;
}

bool ego::gui::DockingSpace::insertTab(const DockingTabPointer& _tab)
{
    if (!_tab || !_tab->getWindow() || findTab(_tab->getWindow()))
    {
        return false;
    }

    const DockingSpacePointer self = ego::StaticPointerCast<DockingSpace>(sharedFromThis());
    if (!self || !attachChild(_tab))
    {
        return false;
    }

    if (!TabAccessor::AttachToSpace(*_tab, self))
    {
        const bool detached = detachChild(_tab);
        EGO_ASSERT(detached);

        return false;
    }

    m_tabs.push_back(_tab);
    m_selectedTabIndex = m_tabs.size() - 1;
    notifyTreeChanged();

    return true;
}

ego::gui::DockingTabPointer ego::gui::DockingSpace::releaseTab(const WindowPointer& _window)
{
    const TabCollection::iterator tabIt = std::find_if(
        m_tabs.begin(),
        m_tabs.end(),
        [&_window](const DockingTabPointer& _tab)
        {
            return _tab && _tab->getWindow().get() == _window.get();
        });
    if (tabIt == m_tabs.end())
    {
        return nullptr;
    }

    const size_t tabIndex = static_cast<size_t>(tabIt - m_tabs.begin());
    const DockingTabPointer tab = *tabIt;
    if (!TabAccessor::IsAttachedToSpace(*tab, *this) || !detachChild(tab))
    {
        return nullptr;
    }

    TabAccessor::DetachFromSpace(*tab);
    m_tabs.erase(tabIt);
    if (m_tabs.empty())
    {
        m_selectedTabIndex = InvalidTabIndex;
    }
    else if (m_selectedTabIndex == tabIndex)
    {
        m_selectedTabIndex = (std::min)(tabIndex, m_tabs.size() - 1);
    }
    else if (m_selectedTabIndex > tabIndex && m_selectedTabIndex != InvalidTabIndex)
    {
        --m_selectedTabIndex;
    }

    notifyTreeChanged();

    return tab;
}

bool ego::gui::DockingSpace::selectWindow(const WindowPointer& _window)
{
    const DockingTabPointer tab = findTab(_window);

    return tab && selectTab(*tab);
}

bool ego::gui::DockingSpace::selectTab(const DockingTab& _tab)
{
    const TabCollection::const_iterator tabIt = std::find_if(
        m_tabs.begin(),
        m_tabs.end(),
        [&_tab](const DockingTabPointer& _currentTab)
        {
            return _currentTab.get() == &_tab;
        });
    if (tabIt == m_tabs.end())
    {
        return false;
    }

    const size_t selectedTabIndex = static_cast<size_t>(tabIt - m_tabs.begin());
    if (m_selectedTabIndex != selectedTabIndex)
    {
        m_selectedTabIndex = selectedTabIndex;
        invalidateLayout();
    }

    return true;
}

bool ego::gui::DockingSpace::isTabSelected(const DockingTab& _tab) const
{
    return m_selectedTabIndex < m_tabs.size() && m_tabs[m_selectedTabIndex].get() == &_tab;
}

bool ego::gui::DockingSpace::hasNonCollapsedWindowExcept(const Window& _window) const
{
    for (const DockingTabPointer& tab : m_tabs)
    {
        const WindowPointer window = tab ? tab->getWindow() : nullptr;
        if (window && window.get() != &_window && !window->isCollapsed())
        {
            return true;
        }
    }

    return false;
}

bool ego::gui::DockingSpace::isEmpty() const
{
    return m_tabs.empty();
}

ego::gui::DockingMetrics ego::gui::DockingSpace::measure(const DockingMeasureContext& _context) const
{
    if (_context.m_excludedSpace && &_context.m_excludedSpace->get() == this)
    {
        return DockingMetrics();
    }

    for (const DockingTabPointer& tab : m_tabs)
    {
        const WindowPointer window = tab ? tab->getWindow() : nullptr;
        if (window && !window->isCollapsed())
        {
            return DockingMetrics{
                .m_spaceCount = 1,
                .m_minimumSize = Size((std::max)(0.0f, _context.m_minimumSpaceSize.m_x), (std::max)(0.0f, _context.m_minimumSpaceSize.m_y)),
            };
        }
    }

    return DockingMetrics();
}

void ego::gui::DockingSpace::clearInteraction()
{
    for (const DockingTabPointer& tab : m_tabs)
    {
        if (tab)
        {
            tab->clearInteraction();
        }
    }
}

bool ego::gui::DockingSpace::isChildActive(size_t _index) const
{
    if (_index >= m_tabs.size() || !m_tabs[_index])
    {
        return false;
    }

    const WindowPointer window = m_tabs[_index]->getWindow();

    return window && window->isVisible();
}

size_t ego::gui::DockingSpace::resolveSelectedTabIndex()
{
    if (m_selectedTabIndex < m_tabs.size())
    {
        const WindowPointer selectedWindow = m_tabs[m_selectedTabIndex] ? m_tabs[m_selectedTabIndex]->getWindow() : nullptr;
        if (selectedWindow && selectedWindow->isVisible())
        {
            return m_selectedTabIndex;
        }
    }

    m_selectedTabIndex = InvalidTabIndex;
    for (size_t tabIndex = 0; tabIndex < m_tabs.size(); ++tabIndex)
    {
        const WindowPointer window = m_tabs[tabIndex] ? m_tabs[tabIndex]->getWindow() : nullptr;
        if (window && window->isVisible())
        {
            m_selectedTabIndex = tabIndex;

            break;
        }
    }

    return m_selectedTabIndex;
}

void ego::gui::DockingSpace::arrangeTabs(const Rect& _bounds, const DockingStyle& _style)
{
    resolveSelectedTabIndex();
    const Rect contentBounds = getContentBounds(_bounds, _style);

    size_t visibleTabCount = 0;
    for (const DockingTabPointer& tab : m_tabs)
    {
        const WindowPointer window = tab ? tab->getWindow() : nullptr;
        visibleTabCount += window && window->isVisible() ? 1 : 0;
    }

    const float tabHeight = (std::min)((std::max)(0.0f, _style.m_tabHeight), _bounds.m_size.m_y);
    const float tabWidth = visibleTabCount > 0 ? _bounds.m_size.m_x / static_cast<float>(visibleTabCount) : 0.0f;
    size_t visibleTabIndex = 0;
    for (size_t tabIndex = 0; tabIndex < m_tabs.size(); ++tabIndex)
    {
        const DockingTabPointer& tab = m_tabs[tabIndex];
        const WindowPointer window = tab ? tab->getWindow() : nullptr;
        const bool isVisible = window && window->isVisible();
        const Rect headerBounds =
            isVisible ? Rect(_bounds.m_position.m_x + tabWidth * static_cast<float>(visibleTabIndex), _bounds.m_position.m_y, tabWidth, tabHeight) :
                        Rect(_bounds.m_position, SizeZero);
        if (tab)
        {
            tab->setArrangement(headerBounds, contentBounds);
        }

        visibleTabIndex += isVisible ? 1 : 0;
    }
}

ego::gui::Rect ego::gui::DockingSpace::getContentBounds(const Rect& _bounds, const DockingStyle& _style) const
{
    const float tabHeight = (std::min)((std::max)(0.0f, _style.m_tabHeight), _bounds.m_size.m_y);

    return Rect(_bounds.m_position.m_x, _bounds.m_position.m_y + tabHeight, _bounds.m_size.m_x, (std::max)(0.0f, _bounds.m_size.m_y - tabHeight));
}

ego::gui::Size ego::gui::DockingSpace::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const Rect bounds(PositionZero, _constraints.m_maximumSize);
    arrangeTabs(bounds, _context.getTheme().m_docking);
    for (const DockingTabPointer& tab : m_tabs)
    {
        if (tab)
        {
            _context.measure(*tab, _constraints);
        }
    }

    return _constraints.m_maximumSize;
}

void ego::gui::DockingSpace::updateGeometry(const LayoutContext& _context)
{
    const Rect& bounds = getLayoutBounds();
    arrangeTabs(bounds, _context.getTheme().m_docking);
    for (const DockingTabPointer& tab : m_tabs)
    {
        if (tab)
        {
            _context.arrange(*tab, bounds);
        }
    }
}

void ego::gui::DockingSpace::drawBaseLayer(PaintContext& _context) const
{
    if (m_tabs.empty())
    {
        return;
    }

    const DockingStyle& style = _context.getTheme().m_docking;
    _context.drawBox(getLayoutBounds(), style.m_background);
    _context.drawBorder(getLayoutBounds(), 1.0f, style.m_spaceBorder);
}

size_t ego::gui::DockingSpace::getChildCount() const
{
    return m_tabs.size();
}

ego::gui::WidgetPointer ego::gui::DockingSpace::getChild(size_t _index) const
{
    return _index < m_tabs.size() ? m_tabs[_index] : nullptr;
}
