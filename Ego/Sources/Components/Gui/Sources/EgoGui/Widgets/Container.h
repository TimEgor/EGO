#pragma once

#include <cstddef>

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class Container : public Widget
    {
    public:
        ~Container() override = default;

        EGO_RTTI_VIRTUAL(Container, Widget);

    protected:
        Container() = default;

        bool isChildHitTestVisible(const Position& _position) const override;

    private:
        size_t getChildCount() const override = 0;
        const WidgetPointer& getChild(size_t _index) const override = 0;
    };

} // namespace ego::gui
