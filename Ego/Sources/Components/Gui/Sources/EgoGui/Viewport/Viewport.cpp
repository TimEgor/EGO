#include "Viewport.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/WindowHost.h"
#include "EgoGui/Input/InputRouter.h"

#include "SurfaceRoot.h"
#include "ViewportProvider.h"

namespace
{
    constexpr size_t MaximumLayoutPassCount = 64;
}

ego::SharedPointer<ego::gui::Viewport> ego::gui::Viewport::ViewportAccessor::Create(ViewportID _id, ViewportRole _role, const Size& _size)
{
    const ego::SharedPointer<Viewport> viewport = new Viewport(_id, _role, _size);
    return viewport && viewport->m_root && viewport->m_inputRouter ? viewport : nullptr;
}

void ego::gui::Viewport::ViewportAccessor::Update(Viewport& _viewport, const LayoutContext& _layoutContext, const ViewportUpdate& _update)
{
    _viewport.update(_layoutContext, _update);
}

void ego::gui::Viewport::ViewportAccessor::ClearInteraction(Viewport& _viewport)
{
    _viewport.clearInteraction();
}

void ego::gui::Viewport::ViewportAccessor::InvalidateLayout(Viewport& _viewport)
{
    _viewport.invalidateLayout();
}

void ego::gui::Viewport::ViewportAccessor::EmitDrawCommands(Viewport& _viewport, const LayoutContext& _layoutContext, PaintContext& _paintContext)
{
    _viewport.emitDrawCommands(_layoutContext, _paintContext);
}

ego::gui::Viewport::Viewport(ViewportID _id, ViewportRole _role, const Size& _size)
    : m_id(_id),
      m_role(_role),
      m_size(_size),
      m_root(SurfaceRoot::Create()),
      m_inputRouter(InputRouter::Create(m_root))
{
}

ego::gui::Viewport::~Viewport()
{
    clearInteraction();
    m_inputRouter = nullptr;
    if (m_root)
    {
        m_root->clearWidgets();
        m_root = nullptr;
    }
}

ego::gui::ViewportID ego::gui::Viewport::getID() const
{
    return m_id;
}

ego::gui::ViewportRole ego::gui::Viewport::getRole() const
{
    return m_role;
}

const ego::gui::Size& ego::gui::Viewport::getSize() const
{
    return m_size;
}

ego::GraphicPresenterPointer ego::gui::Viewport::getGraphicPresenterPointer() const
{
    return m_graphicPresenter;
}

ego::gui::WindowHostPointer ego::gui::Viewport::getWindowHost() const
{
    return m_root ? SurfaceRoot::SurfaceRootAccessor::GetWindowHost(*m_root) : nullptr;
}

void ego::gui::Viewport::setSize(const Size& _size)
{
    if (AreEqual(m_size, _size))
    {
        return;
    }

    m_size = _size;
    invalidateLayout();
}

bool ego::gui::Viewport::add(const WidgetPointer& _widget)
{
    return m_root && m_root->addWidget(_widget);
}

bool ego::gui::Viewport::addWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    if (!add(_window))
    {
        return false;
    }

    if (_placement.m_spaceID == InvalidDockingSpaceID || moveWindow(_window, _placement))
    {
        return true;
    }

    remove(_window);
    return false;
}

ego::gui::WidgetPointer ego::gui::Viewport::remove(const WidgetPointer& _widget)
{
    const WindowHostPointer windowHost = getWindowHost();
    const bool wasHostInteractionActive = windowHost && WindowHost::WindowHostAccessor::IsInteractionActive(*windowHost);
    const WidgetPointer removedWidget = m_root ? m_root->removeWidget(_widget) : nullptr;
    if (removedWidget && m_inputRouter)
    {
        m_inputRouter->cancelPointerCapture(removedWidget);
        if (wasHostInteractionActive)
        {
            m_inputRouter->cancelPointerCapture(windowHost);
        }
    }

    return removedWidget;
}

void ego::gui::Viewport::clear()
{
    if (m_root)
    {
        m_root->clearWidgets();
    }
    if (m_inputRouter)
    {
        m_inputRouter->cancelPointerCapture();
    }
}

const ego::gui::Viewport::WidgetCollection& ego::gui::Viewport::getWidgets() const
{
    static const WidgetCollection EmptyWidgets;
    return m_root ? m_root->getWidgets() : EmptyWidgets;
}

ego::gui::WidgetPointer ego::gui::Viewport::getFocusedWidget() const
{
    return m_inputRouter ? m_inputRouter->getFocusedWidget() : nullptr;
}

bool ego::gui::Viewport::setDockingEnabled(bool _isEnabled)
{
    const WindowHostPointer windowHost = getWindowHost();
    if (!windowHost)
    {
        return false;
    }

    const bool dockingStateChanged = windowHost->isDockingEnabled() != _isEnabled;
    if (!windowHost->setDockingEnabled(_isEnabled))
    {
        return false;
    }

    if (dockingStateChanged && m_inputRouter)
    {
        m_inputRouter->cancelPointerCapture(windowHost);
    }
    return true;
}

bool ego::gui::Viewport::isDockingEnabled() const
{
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost && windowHost->isDockingEnabled();
}

ego::gui::DockingSpaceID ego::gui::Viewport::getDefaultDockingSpaceID() const
{
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost ? windowHost->getDefaultSpaceID() : InvalidDockingSpaceID;
}

ego::gui::DockingSpaceID ego::gui::Viewport::getWindowDockingSpaceID(const WindowPointer& _window) const
{
    const WindowHostPointer windowHost = getWindowHost();
    return windowHost ? windowHost->getWindowSpaceID(_window) : InvalidDockingSpaceID;
}

bool ego::gui::Viewport::moveWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    const WindowHostPointer windowHost = getWindowHost();
    if (!windowHost)
    {
        return false;
    }

    const bool wasInteractionAffected = windowHost->isInteractionAffectedByPlacement(_window, _placement);
    if (!windowHost->placeWindow(_window, _placement))
    {
        return false;
    }

    if (m_inputRouter)
    {
        const WidgetPointer cancellationTarget = wasInteractionAffected ? ego::StaticPointerCast<Widget>(windowHost) : ego::StaticPointerCast<Widget>(_window);
        m_inputRouter->cancelPointerCapture(cancellationTarget);
    }
    return true;
}

void ego::gui::Viewport::update(const LayoutContext& _layoutContext, const ViewportUpdate& _update)
{
    m_graphicPresenter = _update.m_graphicPresenter;
    setSize(_update.m_size);

    if (!stabilizeLayout(_layoutContext))
    {
        return;
    }

    for (const InputEventPointer& input : _update.m_input)
    {
        if (input)
        {
            m_inputRouter->process(*input);
        }
        if (!stabilizeLayout(_layoutContext))
        {
            return;
        }
    }
}

void ego::gui::Viewport::clearInteraction()
{
    const WindowHostPointer windowHost = getWindowHost();
    if (windowHost)
    {
        windowHost->clearInteraction();
    }

    if (m_inputRouter)
    {
        m_inputRouter->clear();
    }
}

void ego::gui::Viewport::invalidateLayout()
{
    if (m_root)
    {
        m_root->invalidateLayout();
    }
}

bool ego::gui::Viewport::stabilizeLayout(const LayoutContext& _layoutContext)
{
    if (!m_root || !m_inputRouter)
    {
        return false;
    }

    const WindowHostPointer windowHost = getWindowHost();
    size_t layoutPassCount = 0;
    while (true)
    {
        while (m_root->isLayoutInvalidated())
        {
            const bool wasHostInteractionActive = windowHost && WindowHost::WindowHostAccessor::IsInteractionActive(*windowHost);
            if (!m_root->updateLayoutIfNeeded(_layoutContext, m_size))
            {
                break;
            }

            m_inputRouter->refreshAfterLayout();
            if (wasHostInteractionActive && !WindowHost::WindowHostAccessor::IsInteractionActive(*windowHost))
            {
                m_inputRouter->cancelPointerCapture(windowHost);
            }
            ++layoutPassCount;
            if (layoutPassCount >= MaximumLayoutPassCount && m_root->isLayoutInvalidated())
            {
                EGO_ASSERT_FAIL_MESSAGE("Layout did not stabilize.");
                return false;
            }
        }

        if (!windowHost || !windowHost->flushWindowNotifications() || !m_root->isLayoutInvalidated())
        {
            return true;
        }
        if (layoutPassCount >= MaximumLayoutPassCount)
        {
            EGO_ASSERT_FAIL_MESSAGE("Layout did not stabilize.");
            return false;
        }
    }
}

void ego::gui::Viewport::emitDrawCommands(const LayoutContext& _layoutContext, PaintContext& _paintContext)
{
    if (!stabilizeLayout(_layoutContext))
    {
        return;
    }

    const SurfaceRoot::TraversalScope treeTraversal(*m_root);
    m_root->emitDrawCommands(_paintContext);
}
