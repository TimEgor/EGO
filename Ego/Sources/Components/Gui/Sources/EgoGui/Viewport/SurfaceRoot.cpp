#include "SurfaceRoot.h"

#include <algorithm>

#include "EgoCore/Assert/Assert.h"

ego::gui::SurfaceRoot::TraversalScope::TraversalScope(SurfaceRoot& _root)
    : m_root(_root)
{
    m_root.beginTraversal();
}

ego::gui::SurfaceRoot::TraversalScope::~TraversalScope()
{
    m_root.endTraversal();
}

ego::gui::SurfaceRoot::~SurfaceRoot()
{
    clearWidgets();
}

ego::gui::SurfaceRootPointer ego::gui::SurfaceRoot::Create()
{
    const SurfaceRootPointer surfaceRoot = new SurfaceRoot();
    surfaceRoot->m_self = surfaceRoot;
    surfaceRoot->bindSurfaceRoot(surfaceRoot, nullptr, surfaceRoot);
    return surfaceRoot;
}

bool ego::gui::SurfaceRoot::addWidget(const WidgetPointer& _widget)
{
    if (!canMutateTree() || !attachChild(_widget))
    {
        return false;
    }

    m_widgets.push_back(_widget);
    notifyTreeChanged();
    return true;
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::removeWidget(const WidgetPointer& _widget)
{
    if (!canMutateTree())
    {
        return nullptr;
    }

    const WidgetCollection::iterator widgetIt = std::find_if(
        m_widgets.begin(),
        m_widgets.end(),
        [&_widget](const WidgetPointer& _currentWidget)
        {
            return _currentWidget.get() == _widget.get();
        });
    if (widgetIt == m_widgets.end())
    {
        return nullptr;
    }

    const WidgetPointer widget = *widgetIt;
    m_widgets.erase(widgetIt);
    if (!detachChild(widget))
    {
        m_widgets.push_back(widget);
        return nullptr;
    }
    notifyTreeChanged();
    return widget;
}

void ego::gui::SurfaceRoot::clearWidgets()
{
    if (!canMutateTree() || m_widgets.empty())
    {
        return;
    }

    WidgetCollection widgets;
    widgets.swap(m_widgets);
    for (const WidgetPointer& widget : widgets)
    {
        detachChild(widget);
    }
    notifyTreeChanged();
}

void ego::gui::SurfaceRoot::bringWidgetToFront(const WidgetPointer& _widget)
{
    if (!canMutateTree())
    {
        return;
    }

    WidgetPointer topLevelWidget = _widget;
    while (topLevelWidget && !topLevelWidget->isDirectChildOf(*this))
    {
        topLevelWidget = topLevelWidget->getParent();
    }
    if (!topLevelWidget || (!m_widgets.empty() && m_widgets.back().get() == topLevelWidget.get()))
    {
        return;
    }

    const WidgetCollection::iterator widgetIt = std::find_if(
        m_widgets.begin(),
        m_widgets.end(),
        [&topLevelWidget](const WidgetPointer& _currentWidget)
        {
            return _currentWidget.get() == topLevelWidget.get();
        });
    if (widgetIt == m_widgets.end())
    {
        return;
    }

    const WidgetPointer widget = *widgetIt;
    m_widgets.erase(widgetIt);
    m_widgets.push_back(widget);

    notifyTreeChanged();
}

const ego::gui::SurfaceRoot::WidgetCollection& ego::gui::SurfaceRoot::getWidgets() const
{
    return m_widgets;
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::findWidgetAt(const Position& _position) const
{
    const SurfaceRootPointer surfaceRoot = m_self.lock();
    if (!surfaceRoot || !isVisible() || !getLayoutBounds().contains(_position))
    {
        return nullptr;
    }

    WidgetPointer currentWidget = surfaceRoot;
    while (currentWidget->isChildHitTestVisible(_position))
    {
        WidgetPointer hitWidget = nullptr;
        const size_t childCount = currentWidget->getChildCount();
        for (size_t childIndex = childCount; childIndex > 0; --childIndex)
        {
            const WidgetPointer& child = currentWidget->getChild(childIndex - 1);
            if (child && child->isVisible() && child->getLayoutBounds().contains(_position) && child->isDirectChildOf(*currentWidget))
            {
                hitWidget = child;
                break;
            }
        }

        if (!hitWidget)
        {
            break;
        }

        currentWidget = hitWidget;
    }

    return currentWidget;
}

bool ego::gui::SurfaceRoot::isInputTarget(const WidgetPointer& _widget) const
{
    if (!_widget || !_widget->isAttachedTo(*this))
    {
        return false;
    }

    WidgetPointer currentWidget = _widget;
    while (currentWidget)
    {
        if (!currentWidget->isVisible())
        {
            return false;
        }
        if (currentWidget.get() == this)
        {
            return true;
        }

        currentWidget = currentWidget->getParent();
    }

    return false;
}

ego::gui::Size ego::gui::SurfaceRoot::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints)
{
    const Rect surfaceBounds(PositionZero, _constraints.m_maximumSize);
    for (const WidgetPointer& widget : m_widgets)
    {
        if (!widget)
        {
            continue;
        }

        const Rect widgetBounds = widget->resolveTopLevelBounds(surfaceBounds);
        widget->updatePreferredSize(_context, LayoutConstraints(widgetBounds.m_size));
    }

    return _constraints.m_maximumSize;
}

void ego::gui::SurfaceRoot::updateGeometry(const LayoutContext& _context)
{
    const Rect& surfaceBounds = getLayoutBounds();
    for (const WidgetPointer& widget : m_widgets)
    {
        if (!widget)
        {
            continue;
        }

        widget->applyLayout(_context, widget->resolveTopLevelBounds(surfaceBounds));
    }
}

void ego::gui::SurfaceRoot::invalidateLayout()
{
    m_layoutInvalidated = true;
}

bool ego::gui::SurfaceRoot::isLayoutInvalidated() const
{
    return m_layoutInvalidated;
}

bool ego::gui::SurfaceRoot::updateLayoutIfNeeded(const LayoutContext& _context, const Size& _size)
{
    if (!m_layoutInvalidated)
    {
        return false;
    }

    const TraversalScope treeTraversal(*this);
    m_layoutInvalidated = false;
    const Rect surfaceBounds(0.0f, 0.0f, _size.m_x, _size.m_y);
    updatePreferredSize(_context, LayoutConstraints(_size));
    applyLayout(_context, surfaceBounds);

    return true;
}

bool ego::gui::SurfaceRoot::canMutateTree() const
{
    return m_traversalDepth == 0;
}

void ego::gui::SurfaceRoot::beginTraversal()
{
    ++m_traversalDepth;
}

void ego::gui::SurfaceRoot::endTraversal()
{
    EGO_ASSERT(m_traversalDepth > 0);
    if (m_traversalDepth > 0)
    {
        --m_traversalDepth;
    }
}

size_t ego::gui::SurfaceRoot::getChildCount() const
{
    return m_widgets.size();
}

const ego::gui::WidgetPointer& ego::gui::SurfaceRoot::getChild(size_t _index) const
{
    return m_widgets[_index];
}
