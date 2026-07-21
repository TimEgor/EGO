#include "Viewport.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"

#include "EgoGui/Input/InputRouter.h"

#include "ViewportBackend.h"

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

void ego::gui::Viewport::setSize(const Size& _size)
{
    if (m_size.m_x == _size.m_x && m_size.m_y == _size.m_y)
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

ego::gui::WidgetPointer ego::gui::Viewport::remove(const WidgetPointer& _widget)
{
    if (!m_root)
    {
        return nullptr;
    }

    const WidgetPointer removedWidget = m_root->removeWidget(_widget);
    return removedWidget;
}

void ego::gui::Viewport::clear()
{
    if (m_root)
    {
        m_root->clearWidgets();
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
    if (!m_inputRouter)
    {
        return;
    }

    m_inputRouter->clear();
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

    size_t layoutPassCount = 0;
    while (m_root->updateLayoutIfNeeded(_layoutContext, m_size))
    {
        m_inputRouter->refreshAfterLayout();
        ++layoutPassCount;
        if (layoutPassCount >= MaximumLayoutPassCount && m_root->isLayoutInvalidated())
        {
            EGO_ASSERT_FAIL_MESSAGE("Layout did not stabilize.");
            return false;
        }
    }

    return true;
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
