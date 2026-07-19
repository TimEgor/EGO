#pragma once

#include <initializer_list>

#include "EgoGui/Widgets/Box.h"

namespace ego::gui
{
    class VerticalPanel;
    EGO_POINTER(VerticalPanel);

    class VerticalPanel final : public Box
    {
    public:
        static VerticalPanelPointer Create();
        static VerticalPanelPointer Create(std::initializer_list<BoxChild> _children);

        EGO_RTTI_VIRTUAL(VerticalPanel, Box);

    private:
        VerticalPanel();
    };
} // namespace ego::gui
