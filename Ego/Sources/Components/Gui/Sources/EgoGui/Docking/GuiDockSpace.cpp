#include "GuiDockSpace.h"

#include <algorithm>

namespace
{
    constexpr float TabHeaderHeight = 28.0f;
    constexpr float TabWidth = 120.0f;
} // namespace

ego::gui::GuiDockSpacePointer ego::gui::GuiDockSpace::Create()
{
    return new GuiDockSpace();
}

bool ego::gui::GuiDockSpace::openTab(const GuiDockTabDesc& _desc)
{
    if (_desc.m_id == InvalidGuiDockTabID || !_desc.m_content)
    {
        return false;
    }

    const TabCollection::iterator existingTab = findTab(_desc.m_id);
    if (existingTab != m_tabs.end())
    {
        m_activeTabID = _desc.m_id;
        return true;
    }

    GuiDockTab tab;
    tab.m_id = _desc.m_id;
    tab.m_title = _desc.m_title;
    tab.m_content = _desc.m_content;
    m_tabs.push_back(tab);
    m_activeTabID = tab.m_id;
    return true;
}

bool ego::gui::GuiDockSpace::closeTab(GuiDockTabID _tabID)
{
    const TabCollection::iterator iter = findTab(_tabID);
    if (iter == m_tabs.end())
    {
        return false;
    }

    m_tabs.erase(iter);
    if (m_activeTabID == _tabID)
    {
        m_activeTabID = m_tabs.empty() ? InvalidGuiDockTabID : m_tabs.front().m_id;
    }

    return true;
}

bool ego::gui::GuiDockSpace::setActiveTab(GuiDockTabID _tabID)
{
    if (findTab(_tabID) == m_tabs.end())
    {
        return false;
    }

    m_activeTabID = _tabID;
    return true;
}

ego::gui::GuiDockLayout ego::gui::GuiDockSpace::saveLayout() const
{
    GuiDockLayout layout;
    layout.m_activeTabID = m_activeTabID;
    return layout;
}

bool ego::gui::GuiDockSpace::restoreLayout(const GuiDockLayout& _layout)
{
    if (_layout.m_activeTabID == InvalidGuiDockTabID)
    {
        m_activeTabID = InvalidGuiDockTabID;
        return true;
    }

    return setActiveTab(_layout.m_activeTabID);
}

ego::gui::GuiReply ego::gui::GuiDockSpace::handleEvent(const GuiInputEvent& _event)
{
    if (!isVisible())
    {
        return GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        const GuiDockTab* tab = findTabAtPosition(_event.m_position);
        m_hoveredTabID = tab ? tab->m_id : InvalidGuiDockTabID;
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left)
    {
        GuiDockTab* tab = findTabAtPosition(_event.m_position);
        if (tab)
        {
            m_activeTabID = tab->m_id;
            return GuiReply::Handled();
        }
    }

    const TabCollection::iterator activeTab = findTab(m_activeTabID);
    const GuiWidgetPointer content = activeTab != m_tabs.end() ? activeTab->m_content : nullptr;
    if (!content || !content->isVisible())
    {
        return GuiReply::Unhandled();
    }

    return content->handleEvent(_event);
}

ego::gui::GuiSize ego::gui::GuiDockSpace::onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize)
{
    const TabCollection::const_iterator activeTab = findTab(m_activeTabID);
    const GuiWidgetPointer content = activeTab != m_tabs.end() ? activeTab->m_content : nullptr;
    if (!content)
    {
        return _availableSize;
    }

    const GuiSize contentAvailableSize(_availableSize.m_x, (std::max)(0.0f, _availableSize.m_y - TabHeaderHeight));
    const GuiSize contentSize = content->measure(_context, contentAvailableSize);
    return GuiSize((std::max)(_availableSize.m_x, contentSize.m_x), contentSize.m_y + TabHeaderHeight);
}

void ego::gui::GuiDockSpace::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    const TabCollection::const_iterator activeTab = findTab(m_activeTabID);
    const GuiWidgetPointer content = activeTab != m_tabs.end() ? activeTab->m_content : nullptr;
    if (!content)
    {
        return;
    }

    const GuiRect contentRect(_rect.m_position.m_x, _rect.m_position.m_y + TabHeaderHeight, _rect.m_size.m_x, (std::max)(0.0f, _rect.m_size.m_y - TabHeaderHeight));
    content->arrange(_context, contentRect);
}

void ego::gui::GuiDockSpace::onPaint(GuiPaintContext& _context) const
{
    _context.drawBox(getRect(), GuiColor(0.075f, 0.080f, 0.090f, 1.0f));

    float tabX = getRect().m_position.m_x;
    for (const GuiDockTab& tab : m_tabs)
    {
        const bool isActive = tab.m_id == m_activeTabID;
        const bool isHovered = tab.m_id == m_hoveredTabID;
        GuiColor tabColor = GuiColor(0.11f, 0.12f, 0.14f, 1.0f);
        if (isHovered)
        {
            tabColor = GuiColor(0.15f, 0.17f, 0.19f, 1.0f);
        }
        if (isActive)
        {
            tabColor = GuiColor(0.18f, 0.20f, 0.23f, 1.0f);
        }

        const GuiRect tabRect(tabX, getRect().m_position.m_y, TabWidth, TabHeaderHeight);
        _context.drawBox(tabRect, tabColor);

        const GuiRect titleRect(tabX + 8.0f, getRect().m_position.m_y + 7.0f, TabWidth - 16.0f, TabHeaderHeight - 8.0f);
        _context.drawText(tab.m_title, titleRect, GuiColorWhite);
        tabX += TabWidth;
    }

    const TabCollection::const_iterator activeTab = findTab(m_activeTabID);
    const GuiWidgetPointer content = activeTab != m_tabs.end() ? activeTab->m_content : nullptr;
    if (content)
    {
        content->paint(_context);
    }
}

ego::gui::GuiDockSpace::TabCollection::iterator ego::gui::GuiDockSpace::findTab(GuiDockTabID _tabID)
{
    return std::find_if(
        m_tabs.begin(),
        m_tabs.end(),
        [_tabID](const GuiDockTab& _tab)
        {
            return _tab.m_id == _tabID;
        });
}

ego::gui::GuiDockSpace::TabCollection::const_iterator ego::gui::GuiDockSpace::findTab(GuiDockTabID _tabID) const
{
    return std::find_if(
        m_tabs.begin(),
        m_tabs.end(),
        [_tabID](const GuiDockTab& _tab)
        {
            return _tab.m_id == _tabID;
        });
}

ego::gui::GuiDockSpace::GuiDockTab* ego::gui::GuiDockSpace::findTabAtPosition(const GuiPosition& _position)
{
    float tabX = getRect().m_position.m_x;
    for (GuiDockTab& tab : m_tabs)
    {
        const GuiRect tabRect(tabX, getRect().m_position.m_y, TabWidth, TabHeaderHeight);
        if (tabRect.contains(_position))
        {
            return &tab;
        }

        tabX += TabWidth;
    }

    return nullptr;
}
