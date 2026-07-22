#include "SurfaceRoot.h"

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/WindowHost.h"

ego::gui::WindowHostPointer ego::gui::SurfaceRoot::SurfaceRootAccessor::GetWindowHost(const SurfaceRoot& _root)
{
    return _root.getWindowHost();
}

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
    if (m_windowHost)
    {
        detachChild(m_windowHost);
        m_windowHost = nullptr;
    }
}

ego::gui::SurfaceRootPointer ego::gui::SurfaceRoot::Create()
{
    const SurfaceRootPointer surfaceRoot = new SurfaceRoot();
    surfaceRoot->bindSurfaceRoot(surfaceRoot);

    const WindowHostPointer windowHost = WindowHost::Create(surfaceRoot);
    if (!windowHost || !surfaceRoot->attachChild(windowHost))
    {
        return nullptr;
    }
    surfaceRoot->m_windowHost = windowHost;
    return surfaceRoot;
}

bool ego::gui::SurfaceRoot::addWidget(const WidgetPointer& _widget)
{
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost && windowHost->addWidget(_widget);
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::removeWidget(const WidgetPointer& _widget)
{
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost ? windowHost->removeWidget(_widget) : nullptr;
}

void ego::gui::SurfaceRoot::clearWidgets()
{
    const WindowHostPointer windowHost = getWindowHost();
    if (windowHost)
    {
        windowHost->clearWidgets();
    }
}

void ego::gui::SurfaceRoot::bringWidgetToFront(const WidgetPointer& _widget)
{
    const WindowHostPointer windowHost = getWindowHost();
    if (windowHost)
    {
        windowHost->bringWidgetToFront(_widget);
    }
}

const ego::gui::SurfaceRoot::WidgetCollection& ego::gui::SurfaceRoot::getWidgets() const
{
    static const WidgetCollection EmptyWidgets;
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost ? windowHost->getWidgets() : EmptyWidgets;
}

ego::gui::WindowHostPointer ego::gui::SurfaceRoot::getWindowHost() const
{
    return m_windowHost ? ego::StaticPointerCast<WindowHost>(m_windowHost) : nullptr;
}

ego::gui::WidgetPointer ego::gui::SurfaceRoot::findWidgetAt(const Position& _position) const
{
    if (!isVisible() || !getLayoutBounds().contains(_position))
    {
        return nullptr;
    }

    WidgetPointer currentWidget = m_windowHost;
    if (!currentWidget || !currentWidget->isVisible() || !currentWidget->getLayoutBounds().contains(_position) || !currentWidget->isDirectChildOf(*this))
    {
        return nullptr;
    }

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
        const Rect& bounds = currentWidget->getLayoutBounds();
        if (!currentWidget->isVisible() || (currentWidget.get() != this && (bounds.m_size.m_x <= 0.0f || bounds.m_size.m_y <= 0.0f)))
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
    if (m_windowHost)
    {
        m_windowHost->updatePreferredSize(_context, _constraints);
    }

    return _constraints.m_maximumSize;
}

void ego::gui::SurfaceRoot::updateGeometry(const LayoutContext& _context)
{
    if (m_windowHost)
    {
        m_windowHost->applyLayout(_context, getLayoutBounds());
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
    return m_windowHost ? 1 : 0;
}

const ego::gui::WidgetPointer& ego::gui::SurfaceRoot::getChild(size_t) const
{
    return m_windowHost;
}
