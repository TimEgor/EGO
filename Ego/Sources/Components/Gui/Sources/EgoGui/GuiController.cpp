#include "EgoGui/GuiController.h"

#include <utility>
#include <vector>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGui/Rendering/PaintContext.h"

ego::gui::GuiController::VisualOperationScope::VisualOperationScope(GuiController& _controller)
    : m_controller(_controller)
{
    EGO_ASSERT(!m_controller.m_isVisualOperationActive);
    m_controller.m_isVisualOperationActive = true;
}

ego::gui::GuiController::VisualOperationScope::~VisualOperationScope()
{
    EGO_ASSERT(m_controller.m_isVisualOperationActive);
    m_controller.m_isVisualOperationActive = false;
}

ego::gui::GuiController::GuiController()
    : m_theme(SharedPointer<Theme>(new Theme(Theme::GetDefault())))
{
}

ego::gui::GuiController::~GuiController()
{
    release();
}

bool ego::gui::GuiController::init(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(ensureVisualOperationInactive());

    const InitData initData = _initData;
    release();

    EGO_CHECK_RETURN_FALSE(initData.m_viewportProvider);

    setTheme(initData.m_theme);
    m_viewportProvider = initData.m_viewportProvider;

    if (!initData.m_fontAtlasDesc.m_fontData.empty())
    {
        m_fontAtlas = FontAtlasPointer(new FontAtlas());
        EGO_CHECK_RETURN_CALL_FALSE(m_fontAtlas && m_fontAtlas->init(initData.m_fontAtlasDesc), release());
    }

    m_isInitialized = true;

    const ViewportPointer primaryViewport = createViewport(ViewportRole::Primary, ViewportDesc());
    if (!primaryViewport)
    {
        release();
        return false;
    }

    return true;
}

void ego::gui::GuiController::release()
{
    if (!ensureVisualOperationInactive())
    {
        return;
    }

    ViewportMap viewports;
    viewports.swap(m_viewports);

    const ViewportProviderPointer viewportProvider = m_viewportProvider;

    m_primaryViewportID = InvalidViewportID;
    m_viewportProvider = nullptr;
    m_fontAtlas = nullptr;
    m_isInitialized = false;

    for (const ViewportMap::value_type& viewportEntry : viewports)
    {
        if (viewportProvider)
        {
            viewportProvider->destroyViewport(viewportEntry.first);
        }

        if (viewportEntry.second)
        {
            Viewport::ViewportAccessor::ClearInteraction(*viewportEntry.second);
        }
    }
}

ego::gui::ViewportPointer ego::gui::GuiController::createViewport(ViewportRole _role, const ViewportDesc& _desc)
{
    const ViewportCreateRequest request(_role, _desc);
    return createViewport(request);
}

ego::gui::ViewportPointer ego::gui::GuiController::createViewport(const ViewportDesc& _desc)
{
    return createViewport(ViewportRole::Secondary, _desc);
}

ego::gui::ViewportPointer ego::gui::GuiController::createViewport(const ViewportCreateRequest& _request)
{
    if (!ensureVisualOperationInactive() || !m_isInitialized || (_request.m_role == ViewportRole::Primary && m_primaryViewportID != InvalidViewportID))
    {
        return nullptr;
    }

    const ViewportID viewportID = prepareNewViewportID();
    if (viewportID == InvalidViewportID)
    {
        return nullptr;
    }

    ViewportCreateRequest request = _request;
    request.m_id = viewportID;

    const ViewportProviderPointer viewportProvider = m_viewportProvider;
    if (!viewportProvider || !viewportProvider->createViewport(request))
    {
        return nullptr;
    }

    ViewportPointer viewport = Viewport::ViewportAccessor::Create(request.m_id, request.m_role, request.m_desc.m_size);
    if (!viewport)
    {
        viewportProvider->destroyViewport(request.m_id);
        return nullptr;
    }

    const std::pair<ViewportMap::iterator, bool> insertResult = m_viewports.emplace(request.m_id, viewport);
    if (!insertResult.second)
    {
        viewportProvider->destroyViewport(request.m_id);
        return nullptr;
    }

    if (request.m_role == ViewportRole::Primary)
    {
        m_primaryViewportID = request.m_id;
    }

    return viewport;
}

bool ego::gui::GuiController::destroyViewport(const ViewportPointer& _viewport)
{
    if (!ensureVisualOperationInactive())
    {
        return false;
    }

    const ViewportPointer viewport = _viewport;
    if (!viewport)
    {
        return false;
    }

    const ViewportID viewportID = viewport->getID();
    const ViewportMap::iterator viewportIt = m_viewports.find(viewportID);
    if (viewportIt == m_viewports.end() || viewportIt->second.getObject() != viewport.getObject())
    {
        return false;
    }

    if (m_primaryViewportID == viewportID)
    {
        m_primaryViewportID = InvalidViewportID;
    }

    m_viewports.erase(viewportIt);

    const ViewportProviderPointer viewportProvider = m_viewportProvider;
    if (viewportProvider)
    {
        viewportProvider->destroyViewport(viewportID);
    }

    Viewport::ViewportAccessor::ClearInteraction(*viewport);

    return true;
}

ego::gui::ViewportPointer ego::gui::GuiController::findViewport(ViewportID _viewportID) const
{
    if (!m_isInitialized || _viewportID == InvalidViewportID)
    {
        return nullptr;
    }

    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second : nullptr;
}

ego::gui::ViewportPointer ego::gui::GuiController::getPrimaryViewport() const
{
    return findViewport(m_primaryViewportID);
}

void ego::gui::GuiController::setTheme(const Theme& _theme)
{
    applyTheme(SharedPointer<Theme>(new Theme(_theme)));
}

ego::gui::ThemePointer ego::gui::GuiController::getTheme() const
{
    return m_theme;
}

void ego::gui::GuiController::update()
{
    if (!m_isInitialized || !m_viewportProvider || !ensureVisualOperationInactive())
    {
        return;
    }

    std::vector<ViewportPointer> viewportsToDestroy;
    {
        const ThemePointer theme = m_theme;
        const VisualOperationScope visualOperation(*this);
        const ViewportProviderPointer viewportProvider = m_viewportProvider;
        const LayoutContext layoutContext{m_fontAtlas, theme};

        for (const auto& [viewportID, viewport] : m_viewports)
        {
            const ViewportUpdate viewportUpdate = viewportProvider->pollViewport(viewportID);

            switch (viewportUpdate.m_status)
            {
            case ViewportUpdateStatus::Alive:
            {
                Viewport::ViewportAccessor::Update(*viewport, layoutContext, viewportUpdate);

                break;
            }

            case ViewportUpdateStatus::CloseRequested:
            case ViewportUpdateStatus::Lost:
            {
                viewportsToDestroy.push_back(viewport);
                break;
            }
            }
        }
    }

    for (const ViewportPointer& viewport : viewportsToDestroy)
    {
        const bool destroyResult = destroyViewport(viewport);
        EGO_ASSERT(destroyResult);
    }
}

ego::gui::GuiRenderData ego::gui::GuiController::buildFrame()
{
    GuiRenderData frame;
    if (!m_isInitialized || !ensureVisualOperationInactive())
    {
        return frame;
    }

    const FontAtlasPointer fontAtlas = m_fontAtlas;
    const ThemePointer theme = m_theme;
    const VisualOperationScope visualOperation(*this);
    const LayoutContext layoutContext{fontAtlas, theme};

    if (fontAtlas && fontAtlas->isInitialized())
    {
        const gpu::TextureViewReference& fontTextureView = fontAtlas->getTextureView();
        if (fontTextureView)
        {
            frame.m_resourceTextureViews.push_back(fontTextureView);
        }
    }

    frame.m_viewports.reserve(m_viewports.size());
    for (const ViewportMap::value_type& viewportEntry : m_viewports)
    {
        const ViewportPointer& viewport = viewportEntry.second;

        ViewportRenderData viewportFrame;
        viewportFrame.m_graphicPresenter = viewport->getGraphicPresenterPointer();

        const Size viewportSize = viewport->getSize();
        if (viewportSize.m_x > 0.0f && viewportSize.m_y > 0.0f)
        {
            viewportFrame.m_drawData.m_viewportSize = viewportSize;

            const Rect viewportRect(0.0f, 0.0f, viewportSize.m_x, viewportSize.m_y);
            PaintContext paintContext(viewportFrame.m_drawData, frame.m_resourceTextureViews, viewportRect, fontAtlas, theme);
            Viewport::ViewportAccessor::EmitDrawCommands(*viewport, layoutContext, paintContext);
        }

        frame.m_viewports.push_back(std::move(viewportFrame));
    }

    return frame;
}

bool ego::gui::GuiController::isInitialized() const
{
    return m_isInitialized;
}

ego::gui::ViewportID ego::gui::GuiController::prepareNewViewportID()
{
    if (m_nextViewportID == InvalidViewportID)
    {
        return InvalidViewportID;
    }

    return m_nextViewportID++;
}

bool ego::gui::GuiController::ensureVisualOperationInactive() const
{
    const bool isInactive = !m_isVisualOperationActive;
    EGO_ASSERT_MESSAGE(isInactive, "Visual operations cannot be reentered or mutate the viewport collection.");

    return isInactive;
}

void ego::gui::GuiController::applyTheme(ThemePointer _theme)
{
    m_theme = std::move(_theme);
    for (const ViewportMap::value_type& viewportEntry : m_viewports)
    {
        if (viewportEntry.second)
        {
            Viewport::ViewportAccessor::InvalidateLayout(*viewportEntry.second);
        }
    }
}
