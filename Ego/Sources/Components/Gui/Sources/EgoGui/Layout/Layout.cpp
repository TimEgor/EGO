#include "Layout.h"

#include <utility>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Theme/Theme.h"
#include "EgoGui/Widgets/Widget.h"

namespace
{
    using WidgetAccessor = ego::gui::Widget::WidgetAccessor;
} // namespace

float ego::gui::Margin::getHorizontal() const
{
    return m_left + m_right;
}

float ego::gui::Margin::getVertical() const
{
    return m_top + m_bottom;
}

ego::gui::LayoutContext::LayoutContext(ego::SharedPointer<FontAtlas> _fontAtlas, ego::SharedPointer<const Theme> _theme)
    : m_fontAtlas(std::move(_fontAtlas)),
      m_theme(std::move(_theme))
{
}

bool ego::gui::LayoutContext::ViewportAccessor::IsLayoutInvalidated(const Widget& _root)
{
    return WidgetAccessor::IsLayoutInvalidated(_root);
}

void ego::gui::LayoutContext::ViewportAccessor::Layout(const LayoutContext& _context, Widget& _root, const Size& _size)
{
    _context.measure(_root, LayoutConstraints(_size));
    _context.arrange(_root, Rect(PositionZero, _size));
    _context.completeLayout(_root);
}

ego::gui::Size ego::gui::LayoutContext::measure(Widget& _widget, const LayoutConstraints& _constraints) const
{
    return WidgetAccessor::UpdatePreferredSize(_widget, *this, _constraints);
}

void ego::gui::LayoutContext::arrange(Widget& _widget, const Rect& _bounds) const
{
    WidgetAccessor::ApplyLayout(_widget, *this, _bounds);
}

const ego::SharedPointer<ego::gui::FontAtlas>& ego::gui::LayoutContext::getFontAtlas() const
{
    return m_fontAtlas;
}

const ego::SharedPointer<const ego::gui::Theme>& ego::gui::LayoutContext::getThemePointer() const
{
    return m_theme;
}

const ego::gui::Theme& ego::gui::LayoutContext::getTheme() const
{
    EGO_ASSERT(m_theme);
    return *m_theme;
}

void ego::gui::LayoutContext::completeLayout(Widget& _root) const
{
    size_t level = 0;
    while (!WidgetAccessor::IsLayoutInvalidated(_root) && completeLayoutLevel(_root, _root, level))
    {
        ++level;
    }
}

bool ego::gui::LayoutContext::completeLayoutLevel(Widget& _widget, const Widget& _root, size_t _level) const
{
    if (WidgetAccessor::IsLayoutInvalidated(_root) || _widget.isCollapsed() || (&_widget != &_root && !_widget.isDescendantOf(_root)))
    {
        return false;
    }

    if (_level == 0)
    {
        WidgetAccessor::OnLayoutCompleted(_widget);

        return true;
    }

    bool hasWidgetAtLevel = false;
    const size_t childCount = WidgetAccessor::GetChildCount(_widget);
    for (size_t childIndex = 0; childIndex < childCount; ++childIndex)
    {
        if (WidgetAccessor::IsLayoutInvalidated(_root))
        {
            break;
        }

        const WidgetPointer child = WidgetAccessor::GetChild(_widget, childIndex);
        if (child && child->isDirectChildOf(_widget) && completeLayoutLevel(*child, _root, _level - 1))
        {
            hasWidgetAtLevel = true;
        }
    }

    return hasWidgetAtLevel;
}
