#include "Layout.h"

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Theme/Theme.h"

float ego::gui::Margin::getHorizontal() const
{
    return m_left + m_right;
}

float ego::gui::Margin::getVertical() const
{
    return m_top + m_bottom;
}

const ego::gui::Theme& ego::gui::LayoutContext::getTheme() const
{
    EGO_ASSERT(m_theme);
    return *m_theme;
}
