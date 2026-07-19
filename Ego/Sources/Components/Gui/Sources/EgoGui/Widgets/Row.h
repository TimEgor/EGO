#pragma once

#include <initializer_list>

#include "EgoGui/Widgets/Box.h"

namespace ego::gui
{
    class Row;
    EGO_POINTER(Row);

    class Row final : public Box
    {
    public:
        static RowPointer Create();
        static RowPointer Create(std::initializer_list<BoxChild> _children);

        EGO_RTTI_VIRTUAL(Row, Box);

    private:
        Row();
    };
} // namespace ego::gui
