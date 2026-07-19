#include "Container.h"

bool ego::gui::Container::isChildHitTestVisible(const Position& _position) const
{
    return getLayoutBounds().contains(_position);
}
