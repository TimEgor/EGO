#include "EgoGui/GuiStyle.h"

ego::gui::GuiStyleChangeID ego::gui::GuiStyle::getChangeID() const
{
    return m_changeID;
}

void ego::gui::GuiStyle::markDirty()
{
    ++m_changeID;
    if (m_changeID == InvalidGuiStyleChangeID)
    {
        ++m_changeID;
    }
}
