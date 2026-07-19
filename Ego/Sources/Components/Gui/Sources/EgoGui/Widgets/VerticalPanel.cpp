#include "VerticalPanel.h"

ego::gui::VerticalPanel::VerticalPanel()
    : Box(Orientation::Vertical, true)
{
}

ego::gui::VerticalPanelPointer ego::gui::VerticalPanel::Create()
{
    return new VerticalPanel();
}

ego::gui::VerticalPanelPointer ego::gui::VerticalPanel::Create(std::initializer_list<BoxChild> _children)
{
    const VerticalPanelPointer panel = Create();
    for (const BoxChild& child : _children)
    {
        if (!panel->addChild(child.m_widget, child.m_slot))
        {
            return nullptr;
        }
    }

    return panel;
}
