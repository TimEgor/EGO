#pragma once

#include <initializer_list>

#include "EgoGui/Widgets/Box.h"

namespace ego::gui
{
    class Column;
    EGO_POINTER(Column);

    class Column final : public Box
    {
    public:
        static ColumnPointer Create();
        static ColumnPointer Create(std::initializer_list<BoxChild> _children);

        EGO_RTTI_VIRTUAL(Column, Box);

    private:
        Column();
    };

} // namespace ego::gui
