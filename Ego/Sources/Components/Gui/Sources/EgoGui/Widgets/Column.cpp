#include "Column.h"

ego::gui::Column::Column()
    : Box(Orientation::Vertical, false)
{
}

ego::gui::ColumnPointer ego::gui::Column::Create()
{
    return new Column();
}

ego::gui::ColumnPointer ego::gui::Column::Create(std::initializer_list<BoxChild> _children)
{
    const ColumnPointer box = Create();
    for (const BoxChild& child : _children)
    {
        if (!box->addChild(child.m_widget, child.m_slot))
        {
            return nullptr;
        }
    }

    return box;
}
