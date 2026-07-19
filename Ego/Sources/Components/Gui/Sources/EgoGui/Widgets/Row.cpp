#include "Row.h"

ego::gui::Row::Row()
    : Box(Orientation::Horizontal, false)
{
}

ego::gui::RowPointer ego::gui::Row::Create()
{
    return new Row();
}

ego::gui::RowPointer ego::gui::Row::Create(std::initializer_list<BoxChild> _children)
{
    const RowPointer box = Create();
    for (const BoxChild& child : _children)
    {
        if (!box->addChild(child.m_widget, child.m_slot))
        {
            return nullptr;
        }
    }

    return box;
}
