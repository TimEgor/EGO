#include "Viewport.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Docking/DockingOverlay.h"

#include "SurfaceRoot.h"
#include "ViewportProvider.h"

ego::SharedPointer<ego::gui::Viewport> ego::gui::Viewport::Create(ViewportID _id, ViewportRole _role, const Size& _size)
{
    const ego::SharedPointer<Viewport> viewport = new Viewport(_id, _role, _size);

    return viewport && viewport->m_root ? viewport : nullptr;
}

ego::gui::Viewport::Viewport(ViewportID _id, ViewportRole _role, const Size& _size)
    : m_id(_id),
      m_role(_role),
      m_size(_size),
      m_root(SurfaceRoot::Create())
{
}

ego::gui::Viewport::~Viewport()
{
    clearInteraction();
    if (m_root)
    {
        m_root->clearWindows();
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

ego::gui::WidgetPointer ego::gui::Viewport::getRootWidget() const
{
    return m_root;
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

bool ego::gui::Viewport::addWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    const WindowPointer window = _window;
    if (!m_root || !m_root->addWindow(window))
    {
        return false;
    }

    if (_placement.m_spaceID == InvalidDockingSpaceID || moveWindow(window, _placement))
    {
        return true;
    }

    removeWindow(window);

    return false;
}

ego::gui::WindowPointer ego::gui::Viewport::removeWindow(const WindowPointer& _window)
{
    const WindowPointer window = _window;
    if (!m_root || !window)
    {
        return nullptr;
    }

    const WindowPointer removedWindow = m_root->removeWindow(window);
    if (removedWindow)
    {
        createUpdateContext().detachInputState(window);
    }

    return removedWindow;
}

void ego::gui::Viewport::clear()
{
    if (m_root)
    {
        createUpdateContext().detachInputState();
        m_root->clearWindows();
    }
}

ego::gui::Viewport::WindowCollection ego::gui::Viewport::getWindows() const
{
    return m_root ? m_root->getWindows() : WindowCollection();
}

ego::gui::WidgetPointer ego::gui::Viewport::getFocusedWidget() const
{
    if (!m_root)
    {
        return nullptr;
    }

    WidgetUpdateContext context(*m_root, m_updateState);

    return context.getFocusedWidget();
}

bool ego::gui::Viewport::setDockingEnabled(bool _isEnabled)
{
    const bool dockingStateChanged = m_root && m_root->isDockingEnabled() != _isEnabled;
    if (!m_root || !m_root->setDockingEnabled(_isEnabled))
    {
        return false;
    }

    if (dockingStateChanged)
    {
        WidgetUpdateContext context = createUpdateContext();
        context.requestPointerCaptureCancellation();
        context.flushPointerCaptureCancellation();
    }

    return true;
}

bool ego::gui::Viewport::isDockingEnabled() const
{
    return m_root && m_root->isDockingEnabled();
}

ego::gui::DockingSpaceID ego::gui::Viewport::getDefaultDockingSpaceID() const
{
    return m_root ? m_root->getDefaultDockingSpaceID() : InvalidDockingSpaceID;
}

ego::gui::DockingSpaceID ego::gui::Viewport::getWindowDockingSpaceID(const WindowPointer& _window) const
{
    return m_root ? m_root->getWindowDockingSpaceID(_window) : InvalidDockingSpaceID;
}

bool ego::gui::Viewport::moveWindow(const WindowPointer& _window, const WindowPlacement& _placement)
{
    const WindowPointer window = _window;
    if (!m_root || !m_root->dockWindow(window, _placement))
    {
        return false;
    }

    WidgetUpdateContext context = createUpdateContext();
    const DockingOverlayPointer dockingOverlay = context.getDockingOverlay();
    if (dockingOverlay)
    {
        dockingOverlay->cancelDrag(window);
    }

    context.requestPointerCaptureCancellation(window);
    context.flushPointerCaptureCancellation();

    return true;
}

void ego::gui::Viewport::update(const LayoutContext& _layoutContext, const ViewportUpdate& _update)
{
    m_graphicPresenter = _update.m_graphicPresenter;
    setSize(_update.m_size);

    WidgetUpdateContext context = createUpdateContext();
    if (!stabilize(_layoutContext, context))
    {
        return;
    }

    for (const InputEventOwner& input : _update.m_input)
    {
        if (!input)
        {
            continue;
        }

        context.update(*input);
        if (!stabilize(_layoutContext, context))
        {
            return;
        }
    }
}

void ego::gui::Viewport::clearInteraction()
{
    if (!m_root)
    {
        return;
    }

    createUpdateContext().clear();
}

void ego::gui::Viewport::invalidateLayout()
{
    if (m_root)
    {
        m_root->invalidateLayout();
    }
}

bool ego::gui::Viewport::stabilize(const LayoutContext& _layoutContext, WidgetUpdateContext& _updateContext)
{
    if (!m_root)
    {
        return false;
    }

    size_t layoutPassCount = 0;
    while (true)
    {
        while (m_root->isLayoutInvalidated())
        {
            if (layoutPassCount >= MaximumLayoutPassCount || !m_root->updateLayoutIfNeeded(_layoutContext, m_size))
            {
                EGO_ASSERT_FAIL_MESSAGE("GUI layout did not stabilize.");

                return false;
            }

            ++layoutPassCount;
        }

        _updateContext.refresh();
        if (!m_root->isLayoutInvalidated())
        {
            return true;
        }

        if (layoutPassCount >= MaximumLayoutPassCount)
        {
            EGO_ASSERT_FAIL_MESSAGE("GUI layout did not stabilize after input state refresh.");

            return false;
        }
    }
}

void ego::gui::Viewport::emitDrawCommands(const LayoutContext& _layoutContext, PaintContext& _paintContext)
{
    if (!m_root)
    {
        return;
    }

    WidgetUpdateContext context = createUpdateContext();
    if (!stabilize(_layoutContext, context))
    {
        return;
    }

    m_root->emitDrawCommands(_paintContext);
}

ego::gui::WidgetUpdateContext ego::gui::Viewport::createUpdateContext()
{
    return WidgetUpdateContext(*m_root, m_updateState);
}
