#include "ViewportManager.h"

#include <algorithm>
#include <cstddef>
#include <memory>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Layout/Layout.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/PaintContext.h"
#include "EgoGui/Widgets/Window.h"

#include "Viewport.h"
#include "ViewportInputContext.h"
#include "ViewportProvider.h"
#include "ViewportDrag.h"

namespace
{
    using ViewportAccessor = ego::gui::Viewport::ViewportManagerAccessor;
    using ViewportInputContextAccessor = ego::gui::ViewportInputContext::ViewportManagerAccessor;
} // namespace

bool ego::gui::ViewportManager::ViewportInputAccessor::BeginWindowDrag(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition,
    const Rect& _localWindowBounds)
{
    return _manager.beginWindowDrag(_inputViewportID, _window, _screenPosition, _localWindowBounds);
}

void ego::gui::ViewportManager::ViewportInputAccessor::UpdateWindowDrag(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition)
{
    _manager.updateWindowDrag(_inputViewportID, _window, _screenPosition);
}

bool ego::gui::ViewportManager::ViewportInputAccessor::FinishWindowDrag(
    ViewportManager& _manager,
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition)
{
    return _manager.finishWindowDrag(_inputViewportID, _window, _screenPosition);
}

void ego::gui::ViewportManager::ViewportInputAccessor::CancelWindowDrag(ViewportManager& _manager, ViewportID _inputViewportID, const WindowPointer& _window)
{
    _manager.cancelWindowDrag(_inputViewportID, _window);
}

bool ego::gui::ViewportManager::ViewportInputAccessor::AllowsInteractionOutsideSurface(const ViewportManager& _manager, ViewportID _inputViewportID)
{
    return _manager.allowsInteractionOutsideSurface(_inputViewportID);
}

ego::gui::ViewportManager::ViewportManager()
    : m_viewportDrag(std::make_unique<ViewportDrag>())
{
}

ego::gui::ViewportManager::~ViewportManager()
{
    release();
}

bool ego::gui::ViewportManager::init(const ViewportProviderPointer& _provider, bool _enableMultiViewport)
{
    EGO_CHECK_INITIALIZATION(!m_provider && m_viewports.empty());
    EGO_CHECK_RETURN_FALSE(_provider);

    m_provider = _provider;

    const ViewportPointer primaryViewport = createViewport(ViewportRole::Primary, ViewportDesc(), false);
    EGO_CHECK_RETURN_CALL_FALSE(primaryViewport, release());

    setMultiViewportEnabled(_enableMultiViewport);

    return true;
}

void ego::gui::ViewportManager::release()
{
    m_viewportDrag->reset(*this);
    destroyViewports();

    m_provider = nullptr;
    m_primaryViewport = nullptr;
    m_nextViewportID = FirstViewportID;
    m_isMultiViewportEnabled = false;
}

bool ego::gui::ViewportManager::update(const LayoutContext& _layoutContext)
{
    validateWindowDrag();
    if (!m_provider)
    {
        return false;
    }

    updateWindowViewports();

    size_t viewportIndex = 0;
    while (viewportIndex < m_viewports.size())
    {
        const ViewportPointer viewport = m_viewports[viewportIndex].m_viewport;
        EGO_CHECK_RETURN_FALSE(viewport);

        const ViewportID viewportID = ViewportAccessor::GetID(*viewport);
        const ViewportUpdate viewportUpdate = m_provider->pollViewport(viewportID);
        if (viewportUpdate.m_status != ViewportUpdateStatus::Alive)
        {
            if (m_primaryViewport.getObject() == viewport.getObject())
            {
                return false;
            }

            const bool recoverWindows = viewportUpdate.m_status == ViewportUpdateStatus::Lost;
            const bool destroyResult = destroyViewport(viewport, recoverWindows);
            EGO_ASSERT(destroyResult);
            continue;
        }

        if (shouldDestroyManagedViewport(viewport))
        {
            recoverViewportWindows(viewport);
            const bool destroyResult = destroyViewport(viewport, false);
            EGO_ASSERT(destroyResult);
            continue;
        }

        applyManagedViewportUpdate(viewport, viewportUpdate);
        ViewportAccessor::ApplyViewportUpdate(*viewport, viewportUpdate);

        if (ViewportAccessor::UpdateLayout(*viewport, _layoutContext))
        {
            for (const InputEventOwner& input : viewportUpdate.m_input)
            {
                if (!input)
                {
                    continue;
                }

                ViewportInputContext inputContext = ViewportInputContextAccessor::Create(*this, viewportID);
                ViewportAccessor::ProcessInput(*viewport, *input, inputContext);
                completeSurfaceInput(viewportID);

                if (!containsViewport(viewport))
                {
                    break;
                }

                if (!ViewportAccessor::UpdateLayout(*viewport, _layoutContext))
                {
                    break;
                }
            }
        }

        if (!containsViewport(viewport))
        {
            continue;
        }

        if (shouldDestroyManagedViewport(viewport))
        {
            recoverViewportWindows(viewport);
            const bool destroyResult = destroyViewport(viewport, false);
            EGO_ASSERT(destroyResult);
            continue;
        }

        syncManagedViewport(viewport);
        updateWindowViewports();

        if (viewportIndex < m_viewports.size() && m_viewports[viewportIndex].m_viewport.getObject() == viewport.getObject())
        {
            ++viewportIndex;
        }
    }

    validateWindowDrag();

    return true;
}

ego::gui::GuiRenderData ego::gui::ViewportManager::buildFrame(const LayoutContext& _layoutContext)
{
    validateWindowDrag();

    GuiRenderData frame;
    if (!m_provider)
    {
        return frame;
    }

    const FontAtlasPointer fontAtlas = _layoutContext.getFontAtlas();
    if (fontAtlas && fontAtlas->isInitialized())
    {
        const gpu::TextureViewReference& fontTextureView = fontAtlas->getTextureView();
        if (fontTextureView)
        {
            frame.m_resourceTextureViews.push_back(fontTextureView);
        }
    }

    frame.m_viewports.reserve(m_viewports.size());
    for (ViewportRecord& viewportRecord : m_viewports)
    {
        const ViewportPointer viewport = viewportRecord.m_viewport;
        EGO_ASSERT(viewport);

        ViewportRenderData viewportFrame;
        viewportFrame.m_graphicPresenter = ViewportAccessor::GetGraphicPresenterPointer(*viewport);

        const Size viewportSize = ViewportAccessor::GetSize(*viewport);
        if (viewportSize.m_x > 0.0f && viewportSize.m_y > 0.0f)
        {
            viewportFrame.m_drawData.m_viewportSize = viewportSize;

            if (ViewportAccessor::UpdateLayout(*viewport, _layoutContext))
            {
                const Rect viewportRect(0.0f, 0.0f, viewportSize.m_x, viewportSize.m_y);
                PaintContext paintContext(
                    viewportFrame.m_drawData,
                    frame.m_resourceTextureViews,
                    viewportRect,
                    _layoutContext.getFontAtlas(),
                    _layoutContext.getThemePointer());
                ViewportAccessor::Paint(*viewport, paintContext);
            }
        }

        frame.m_viewports.push_back(std::move(viewportFrame));
    }

    validateWindowDrag();

    return frame;
}

ego::gui::ViewportPointer ego::gui::ViewportManager::createViewport(const ViewportDesc& _desc)
{
    const ViewportPointer viewport = createViewport(ViewportRole::Secondary, _desc, false);
    if (!viewport)
    {
        return nullptr;
    }

    if (!m_provider->showViewport(ViewportAccessor::GetID(*viewport), false))
    {
        const bool destroyed = destroyViewport(viewport, false);
        EGO_ASSERT(destroyed);

        return nullptr;
    }

    return viewport;
}

bool ego::gui::ViewportManager::destroyViewport(const ViewportPointer& _viewport)
{
    return destroyViewport(_viewport, true);
}

ego::gui::ViewportPointer ego::gui::ViewportManager::getPrimaryViewport() const
{
    return m_primaryViewport;
}

ego::gui::ViewportPointer ego::gui::ViewportManager::findViewport(const WindowPointer& _window) const
{
    if (!_window)
    {
        return nullptr;
    }

    for (const ViewportRecord& viewportRecord : m_viewports)
    {
        const ViewportPointer viewport = viewportRecord.m_viewport;
        if (viewport && ViewportAccessor::ContainsWindow(*viewport, _window))
        {
            return viewport;
        }
    }

    return nullptr;
}

void ego::gui::ViewportManager::setMultiViewportEnabled(bool _isEnabled)
{
    if (m_isMultiViewportEnabled == _isEnabled)
    {
        return;
    }

    m_isMultiViewportEnabled = _isEnabled;
    if (m_isMultiViewportEnabled)
    {
        updateWindowViewports();
    }
    else
    {
        m_viewportDrag->reset(*this);
        mergeManagedViewports();
    }
}

bool ego::gui::ViewportManager::isMultiViewportEnabled() const
{
    return m_isMultiViewportEnabled;
}

void ego::gui::ViewportManager::invalidateLayouts()
{
    for (ViewportRecord& viewportRecord : m_viewports)
    {
        const ViewportPointer viewport = viewportRecord.m_viewport;
        if (viewport)
        {
            ViewportAccessor::InvalidateLayout(*viewport);
        }
    }
}

ego::gui::ViewportPointer ego::gui::ViewportManager::createViewport(ViewportRole _role, const ViewportDesc& _desc, bool _isManaged)
{
    if (_role == ViewportRole::Primary && m_primaryViewport)
    {
        return nullptr;
    }

    if (!m_provider || m_nextViewportID == InvalidViewportID)
    {
        return nullptr;
    }

    const ViewportID viewportID = m_nextViewportID++;
    ViewportCreateRequest request(_role, _desc);
    request.m_id = viewportID;
    if (!m_provider->createViewport(request))
    {
        return nullptr;
    }

    const ViewportUpdate initialUpdate = m_provider->pollViewport(viewportID);
    if (initialUpdate.m_status != ViewportUpdateStatus::Alive)
    {
        m_provider->destroyViewport(viewportID);

        return nullptr;
    }

    ViewportDesc viewportDesc = _desc;
    viewportDesc.m_position = initialUpdate.m_position;
    viewportDesc.m_size = initialUpdate.m_size;

    const ViewportPointer viewport = ViewportAccessor::Create(viewportID, viewportDesc);
    if (!viewport)
    {
        m_provider->destroyViewport(viewportID);

        return nullptr;
    }

    m_viewports.push_back({viewport, _isManaged});

    if (_role == ViewportRole::Primary)
    {
        m_primaryViewport = viewport;
    }

    return viewport;
}

bool ego::gui::ViewportManager::destroyViewport(const ViewportPointer& _viewport, bool _recoverWindows)
{
    const ViewportPointer viewport = _viewport;
    if (!viewport || !containsViewport(viewport) || m_primaryViewport.getObject() == viewport.getObject())
    {
        return false;
    }

    m_viewportDrag->handleViewportDestroying(*this, viewport);
    if (_recoverWindows)
    {
        recoverViewportWindows(viewport);
    }
    else
    {
        ViewportAccessor::Clear(*viewport);
    }

    const ViewportID viewportID = ViewportAccessor::GetID(*viewport);
    const size_t viewportIndex = findViewportIndex(viewportID);
    if (viewportIndex >= m_viewports.size() || m_viewports[viewportIndex].m_viewport.getObject() != viewport.getObject())
    {
        return false;
    }

    m_viewports.erase(m_viewports.begin() + static_cast<std::ptrdiff_t>(viewportIndex));
    m_provider->destroyViewport(viewportID);

    return true;
}

void ego::gui::ViewportManager::destroyViewports()
{
    m_primaryViewport = nullptr;

    while (!m_viewports.empty())
    {
        const ViewportPointer viewport = m_viewports.back().m_viewport;
        if (viewport && m_provider)
        {
            m_provider->destroyViewport(ViewportAccessor::GetID(*viewport));
        }

        m_viewports.pop_back();
    }
}

size_t ego::gui::ViewportManager::findViewportIndex(ViewportID _viewportID) const
{
    const ViewportCollection::const_iterator viewportIt = std::find_if(
        m_viewports.begin(),
        m_viewports.end(),
        [_viewportID](const ViewportRecord& _viewportRecord)
        {
            const ViewportPointer viewport = _viewportRecord.m_viewport;
            return viewport && ViewportAccessor::GetID(*viewport) == _viewportID;
        });

    return static_cast<size_t>(viewportIt - m_viewports.begin());
}

ego::gui::ViewportPointer ego::gui::ViewportManager::findViewport(ViewportID _viewportID) const
{
    if (_viewportID == InvalidViewportID)
    {
        return nullptr;
    }

    const size_t viewportIndex = findViewportIndex(_viewportID);
    return viewportIndex < m_viewports.size() ? m_viewports[viewportIndex].m_viewport : nullptr;
}

bool ego::gui::ViewportManager::containsViewport(const ViewportPointer& _viewport) const
{
    if (!_viewport)
    {
        return false;
    }

    return findViewport(ViewportAccessor::GetID(*_viewport)).getObject() == _viewport.getObject();
}

bool ego::gui::ViewportManager::isManagedViewport(const ViewportPointer& _viewport) const
{
    if (!_viewport)
    {
        return false;
    }

    const size_t viewportIndex = findViewportIndex(ViewportAccessor::GetID(*_viewport));
    return viewportIndex < m_viewports.size() && m_viewports[viewportIndex].m_viewport.getObject() == _viewport.getObject() &&
           m_viewports[viewportIndex].m_isManaged;
}

void ego::gui::ViewportManager::applyManagedViewportUpdate(const ViewportPointer& _viewport, const ViewportUpdate& _update)
{
    if (!isManagedViewport(_viewport) || !_update.m_sizeChanged)
    {
        return;
    }

    const WindowPointer window = ViewportAccessor::GetWindow(*_viewport, 0);
    if (window)
    {
        window->setBounds(Rect(PositionZero, _update.m_size));
    }
}

bool ego::gui::ViewportManager::shouldDestroyManagedViewport(const ViewportPointer& _viewport) const
{
    if (!isManagedViewport(_viewport))
    {
        return false;
    }

    EGO_ASSERT(ViewportAccessor::GetWindowCount(*_viewport) <= 1);
    const WindowPointer window = ViewportAccessor::GetWindow(*_viewport, 0);

    return !window || !window->isVisible();
}

void ego::gui::ViewportManager::updateWindowViewports()
{
    if (!m_isMultiViewportEnabled)
    {
        return;
    }

    size_t viewportIndex = 0;
    while (viewportIndex < m_viewports.size())
    {
        const ViewportPointer viewport = m_viewports[viewportIndex].m_viewport;
        if (!viewport || isManagedViewport(viewport))
        {
            ++viewportIndex;
            continue;
        }

        size_t windowIndex = 0;
        while (windowIndex < ViewportAccessor::GetFloatingWindowCount(*viewport))
        {
            const WindowPointer window = ViewportAccessor::GetFloatingWindow(*viewport, windowIndex);
            if (!window)
            {
                EGO_ASSERT_FAIL_MESSAGE("Viewport contains an invalid window.");
                ++windowIndex;
                continue;
            }

            if (ViewportAccessor::IsWindowBound(*viewport, window) || !window->isVisible() || ViewportAccessor::IsWindowDragActive(*viewport, window) ||
                isWindowDragInput(ViewportAccessor::GetID(*viewport), window))
            {
                ++windowIndex;
                continue;
            }

            const Rect screenBounds = getWindowScreenBounds(window, viewport);
            if (!createWindowViewport(window, viewport, screenBounds))
            {
                EGO_ASSERT_FAIL_MESSAGE("Failed to create a viewport for a floating window.");
                ++windowIndex;
            }
        }

        ++viewportIndex;
    }
}

void ego::gui::ViewportManager::mergeManagedViewports()
{
    size_t viewportIndex = 0;
    while (viewportIndex < m_viewports.size())
    {
        const ViewportPointer viewport = m_viewports[viewportIndex].m_viewport;
        if (!isManagedViewport(viewport))
        {
            ++viewportIndex;
            continue;
        }

        recoverViewportWindows(viewport);
        const bool destroyResult = destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);
    }
}

void ego::gui::ViewportManager::syncManagedViewport(const ViewportPointer& _viewport)
{
    const ViewportPointer viewport = _viewport;
    if (!isManagedViewport(viewport))
    {
        return;
    }

    if (viewport->isDockingEnabled())
    {
        const bool dockingDisabled = viewport->setDockingEnabled(false);
        EGO_ASSERT(dockingDisabled);
    }

    EGO_ASSERT(ViewportAccessor::GetWindowCount(*viewport) <= 1);
    const WindowPointer window = ViewportAccessor::GetWindow(*viewport, 0);
    if (!window)
    {
        return;
    }

    EGO_ASSERT(ViewportAccessor::IsWindowFloating(*viewport, window));
    Rect bounds = window->getFloatingBounds();
    if (!ViewportAccessor::IsWindowDragActive(*viewport, window) && !AreEqual(bounds.m_position, PositionZero))
    {
        const Position& viewportPosition = ViewportAccessor::GetPosition(*viewport);
        Position position(viewportPosition.m_x + bounds.m_position.m_x, viewportPosition.m_y + bounds.m_position.m_y);
        if (m_provider->setViewportPosition(ViewportAccessor::GetID(*viewport), position))
        {
            ViewportAccessor::SetPosition(*viewport, position);
            bounds.m_position = PositionZero;
            window->setBounds(bounds);
        }
    }

    if (bounds.m_size.m_x > 0.0f && bounds.m_size.m_y > 0.0f && !AreEqual(bounds.m_size, ViewportAccessor::GetSize(*viewport)))
    {
        Size size = bounds.m_size;
        if (m_provider->setViewportSize(ViewportAccessor::GetID(*viewport), size))
        {
            ViewportAccessor::SetSize(*viewport, size);
            bounds.m_size = size;
            window->setBounds(bounds);
        }
    }
}

bool ego::gui::ViewportManager::createWindowViewport(const WindowPointer& _window, const ViewportPointer& _sourceViewport, const Rect& _screenBounds)
{
    const WindowPointer window = _window;
    const ViewportPointer sourceViewport = _sourceViewport;
    EGO_CHECK_RETURN_FALSE(window && sourceViewport && ViewportAccessor::ContainsWindow(*sourceViewport, window));
    EGO_CHECK_RETURN_FALSE(!ViewportAccessor::IsWindowBound(*sourceViewport, window) && ViewportAccessor::IsWindowFloating(*sourceViewport, window));

    Size viewportSize = _screenBounds.m_size;
    if (viewportSize.m_x <= 0.0f || viewportSize.m_y <= 0.0f)
    {
        viewportSize = Size(DefaultManagedViewportExtent, DefaultManagedViewportExtent);
    }

    ViewportDesc viewportDesc;
    viewportDesc.m_title = window->getTitle();
    viewportDesc.m_position = _screenBounds.m_position;
    viewportDesc.m_size = viewportSize;

    const ViewportPointer viewport = createViewport(ViewportRole::Secondary, viewportDesc, true);
    EGO_CHECK_RETURN_FALSE(viewport);

    const Rect sourceBounds = window->getFloatingBounds();
    if (!sourceViewport->removeWindow(window))
    {
        const bool destroyResult = destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        return false;
    }

    const Size createdViewportSize = ViewportAccessor::GetSize(*viewport);
    if (createdViewportSize.m_x <= 0.0f || createdViewportSize.m_y <= 0.0f)
    {
        const bool destroyResult = destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        const bool restored = sourceViewport->addWindow(window);
        EGO_ASSERT(restored);

        return false;
    }

    window->setBounds(Rect(PositionZero, createdViewportSize));
    if (!viewport->addWindow(window))
    {
        window->setBounds(sourceBounds);
        const bool restored = sourceViewport->addWindow(window);
        EGO_ASSERT(restored);

        const bool destroyResult = destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        return false;
    }

    if (!m_provider->showViewport(ViewportAccessor::GetID(*viewport), false))
    {
        const bool detached = viewport->removeWindow(window);
        EGO_ASSERT(detached);

        window->setBounds(sourceBounds);
        const bool restored = sourceViewport->addWindow(window);
        EGO_ASSERT(restored);

        const bool destroyResult = destroyViewport(viewport, false);
        EGO_ASSERT(destroyResult);

        return false;
    }

    if (isManagedViewport(sourceViewport) && ViewportAccessor::GetWindowCount(*sourceViewport) == 0)
    {
        const bool destroyResult = destroyViewport(sourceViewport, false);
        EGO_ASSERT(destroyResult);
    }

    return true;
}

void ego::gui::ViewportManager::recoverViewportWindows(const ViewportPointer& _viewport)
{
    if (!_viewport || !m_primaryViewport || _viewport.getObject() == m_primaryViewport.getObject())
    {
        return;
    }

    while (ViewportAccessor::GetWindowCount(*_viewport) > 0)
    {
        const WindowPointer window = ViewportAccessor::GetWindow(*_viewport, 0);
        if (!window)
        {
            EGO_ASSERT_FAIL_MESSAGE("Viewport contains an invalid window.");
            break;
        }

        if (ViewportAccessor::IsWindowBound(*_viewport, window))
        {
            const bool removed = _viewport->removeWindow(window);
            EGO_ASSERT(removed);
            continue;
        }

        const Rect screenBounds = getWindowScreenBounds(window, _viewport);
        const bool removed = _viewport->removeWindow(window);
        if (!removed)
        {
            EGO_ASSERT_FAIL_MESSAGE("Failed to remove a window while recovering a viewport.");
            break;
        }

        const Position& primaryPosition = ViewportAccessor::GetPosition(*m_primaryViewport);
        const Position position(screenBounds.m_position.m_x - primaryPosition.m_x, screenBounds.m_position.m_y - primaryPosition.m_y);
        window->setBounds(Rect(position, screenBounds.m_size));

        const bool added = m_primaryViewport->addWindow(window);
        EGO_ASSERT(added);
    }
}

ego::gui::Rect ego::gui::ViewportManager::getWindowScreenBounds(const WindowPointer& _window, const ViewportPointer& _viewport) const
{
    if (!_window || !_viewport)
    {
        return Rect();
    }

    const Rect bounds = _window->getFloatingBounds();
    const Position& viewportPosition = ViewportAccessor::GetPosition(*_viewport);
    const Position position(viewportPosition.m_x + bounds.m_position.m_x, viewportPosition.m_y + bounds.m_position.m_y);

    return Rect(position, bounds.m_size);
}

bool ego::gui::ViewportManager::beginWindowDrag(
    ViewportID _inputViewportID,
    const WindowPointer& _window,
    const Position& _screenPosition,
    const Rect& _localWindowBounds)
{
    return m_viewportDrag->begin(*this, _inputViewportID, _window, _screenPosition, _localWindowBounds);
}

void ego::gui::ViewportManager::updateWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition)
{
    m_viewportDrag->update(*this, _inputViewportID, _window, _screenPosition);
}

bool ego::gui::ViewportManager::finishWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window, const Position& _screenPosition)
{
    return m_viewportDrag->finish(*this, _inputViewportID, _window, _screenPosition);
}

void ego::gui::ViewportManager::cancelWindowDrag(ViewportID _inputViewportID, const WindowPointer& _window)
{
    m_viewportDrag->cancel(*this, _inputViewportID, _window);
}

bool ego::gui::ViewportManager::allowsInteractionOutsideSurface(ViewportID _inputViewportID) const
{
    return m_viewportDrag->allowsInteractionOutsideSurface(*this, _inputViewportID);
}

void ego::gui::ViewportManager::completeSurfaceInput(ViewportID _inputViewportID)
{
    validateWindowDrag();
    m_viewportDrag->completeInput(*this, _inputViewportID);
    validateWindowDrag();
}

bool ego::gui::ViewportManager::isWindowDragInput(ViewportID _inputViewportID, const WindowPointer& _window) const
{
    return m_viewportDrag->isInput(_inputViewportID, _window);
}

void ego::gui::ViewportManager::validateWindowDrag()
{
    if (!m_viewportDrag->isValid(*this))
    {
        m_viewportDrag->reset(*this);
    }
}
