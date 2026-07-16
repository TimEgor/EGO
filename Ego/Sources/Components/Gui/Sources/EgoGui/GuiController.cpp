#include "EgoGui/GuiController.h"

#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoGui/Rendering/GuiPaintContext.h"

ego::gui::GuiController::~GuiController()
{
    release();
}

bool ego::gui::GuiController::init(const InitData& _initData)
{
    release();

    EGO_CHECK_RETURN_FALSE(_initData.m_viewportBackend);

    m_viewportBackend = _initData.m_viewportBackend;

    m_fontAtlas = GuiFontAtlasPointer(new GuiFontAtlas());
    const GraphicDevicePointer graphicDevice = gpu::GetGraphicDevicePointer();
    EGO_CHECK_RETURN_CALL_FALSE(graphicDevice && m_fontAtlas && m_fontAtlas->init(*graphicDevice, _initData.m_fontAtlasDesc), release());

    m_isInitialized = true;

    const GuiViewportID primaryViewportID = createViewport(GuiViewportRole::Primary, _initData.m_primaryViewportDesc);
    EGO_CHECK_RETURN_CALL_FALSE(primaryViewportID != InvalidGuiViewportID, release());

    return true;
}

void ego::gui::GuiController::release()
{
    if (m_viewportBackend)
    {
        for (const ViewportMap::value_type& viewportEntry : m_viewports)
        {
            if (viewportEntry.second)
            {
                GuiInputEvent focusLostEvent;
                focusLostEvent.m_type = GuiInputEventType::FocusLost;
                viewportEntry.second->processEvent(focusLostEvent);
            }
            m_viewportBackend->destroyViewport(viewportEntry.first);
        }
    }

    m_viewports.clear();
    m_primaryViewportID = InvalidGuiViewportID;
    m_nextViewportID = FirstViewportID;
    m_viewportBackend = nullptr;
    m_fontAtlas = nullptr;
    m_isInitialized = false;
}

ego::gui::GuiViewportID ego::gui::GuiController::createViewport(GuiViewportRole _role, const GuiViewportDesc& _desc)
{
    const GuiViewportCreateRequest request(_role, _desc);
    return createViewport(request);
}

ego::gui::GuiViewportID ego::gui::GuiController::createViewport(const GuiViewportDesc& _desc)
{
    return createViewport(GuiViewportRole::Secondary, _desc);
}

ego::gui::GuiViewportID ego::gui::GuiController::createViewport(const GuiViewportCreateRequest& _request)
{
    if (!m_isInitialized || (_request.m_role == GuiViewportRole::Primary && m_primaryViewportID != InvalidGuiViewportID))
    {
        return InvalidGuiViewportID;
    }

    const GuiViewportID viewportID = prepareNewViewportID();
    if (viewportID == InvalidGuiViewportID)
    {
        return InvalidGuiViewportID;
    }

    GuiViewportCreateRequest request = _request;
    request.m_id = viewportID;

    if (!m_viewportBackend->createViewport(request))
    {
        return InvalidGuiViewportID;
    }

    GuiViewportPointer viewport = new GuiViewport(request.m_id, request.m_role, request.m_desc.m_size);
    if (!viewport)
    {
        m_viewportBackend->destroyViewport(request.m_id);
        return InvalidGuiViewportID;
    }

    const std::pair<ViewportMap::iterator, bool> insertResult = m_viewports.emplace(request.m_id, viewport);
    if (!insertResult.second)
    {
        m_viewportBackend->destroyViewport(request.m_id);
        return InvalidGuiViewportID;
    }

    if (request.m_role == GuiViewportRole::Primary)
    {
        m_primaryViewportID = request.m_id;
    }

    return request.m_id;
}

bool ego::gui::GuiController::destroyViewport(GuiViewportID _viewportID)
{
    const ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end())
    {
        return false;
    }

    if (viewportIt->second)
    {
        GuiInputEvent focusLostEvent;
        focusLostEvent.m_type = GuiInputEventType::FocusLost;
        viewportIt->second->processEvent(focusLostEvent);
    }

    m_viewportBackend->destroyViewport(_viewportID);
    if (m_primaryViewportID == _viewportID)
    {
        m_primaryViewportID = InvalidGuiViewportID;
    }

    m_viewports.erase(viewportIt);
    return true;
}

ego::gui::GuiViewportPointer ego::gui::GuiController::findViewport(GuiViewportID _viewportID) const
{
    if (!m_isInitialized || _viewportID == InvalidGuiViewportID)
    {
        return nullptr;
    }

    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second : nullptr;
}

ego::gui::GuiViewportPointer ego::gui::GuiController::getPrimaryViewport() const
{
    return findViewport(m_primaryViewportID);
}

void ego::gui::GuiController::update()
{
    if (!m_isInitialized)
    {
        return;
    }

    for (ViewportMap::iterator viewportIt = m_viewports.begin(); viewportIt != m_viewports.end();)
    {
        const GuiViewportID viewportID = viewportIt->first;
        const GuiViewportPointer& viewport = viewportIt->second;
        if (!viewport)
        {
            if (m_primaryViewportID == viewportID)
            {
                m_primaryViewportID = InvalidGuiViewportID;
            }
            viewportIt = m_viewports.erase(viewportIt);
            continue;
        }

        GuiViewportUpdate viewportUpdate;
        const bool hasViewportUpdate = m_viewportBackend->updateViewport(viewportID, viewportUpdate);
        if (!hasViewportUpdate || viewportUpdate.m_closeRequested)
        {
            GuiInputEvent focusLostEvent;
            focusLostEvent.m_type = GuiInputEventType::FocusLost;
            viewport->processEvent(focusLostEvent);

            if (hasViewportUpdate)
            {
                m_viewportBackend->destroyViewport(viewportID);
            }
            if (m_primaryViewportID == viewportID)
            {
                m_primaryViewportID = InvalidGuiViewportID;
            }
            viewportIt = m_viewports.erase(viewportIt);
            continue;
        }

        viewport->setSize(viewportUpdate.m_size);
        for (const GuiInputEvent& event : viewportUpdate.m_events)
        {
            viewport->processEvent(event);
        }

        ++viewportIt;
    }
}

ego::gui::GuiFrame ego::gui::GuiController::buildFrame()
{
    GuiFrame frame;
    frame.m_primaryViewportID = m_primaryViewportID;
    frame.m_fontAtlas = m_fontAtlas;
    frame.m_viewports.reserve(m_viewports.size());

    for (const ViewportMap::value_type& viewportEntry : m_viewports)
    {
        const GuiViewportPointer& viewport = viewportEntry.second;
        if (!viewport)
        {
            continue;
        }

        GuiViewportFrame viewportFrame;
        viewportFrame.m_viewportID = viewportEntry.first;

        const GuiSize& viewportSize = viewport->getSize();
        if (viewportSize.m_x > 0.0f && viewportSize.m_y > 0.0f)
        {
            viewportFrame.m_drawData.m_viewportSize = viewportSize;

            GuiLayoutContext layoutContext;
            layoutContext.m_fontAtlas = m_fontAtlas;

            const GuiRect viewportRect(0.0f, 0.0f, viewportSize.m_x, viewportSize.m_y);
            GuiPaintContext paintContext(viewportFrame.m_drawData, viewportRect, m_fontAtlas);
            viewport->buildDrawData(layoutContext, paintContext);
        }

        frame.m_viewports.push_back(std::move(viewportFrame));
    }

    return frame;
}

ego::gui::GuiEventResult ego::gui::GuiController::processEvent(GuiViewportID _viewportID, const GuiInputEvent& _event)
{
    const GuiViewportPointer viewport = findViewport(_viewportID);
    if (!viewport)
    {
        return GuiEventResult::Unhandled;
    }

    return viewport->processEvent(_event);
}

bool ego::gui::GuiController::isInitialized() const
{
    return m_isInitialized;
}

ego::gui::GuiViewportID ego::gui::GuiController::prepareNewViewportID()
{
    if (m_nextViewportID == InvalidGuiViewportID)
    {
        return InvalidGuiViewportID;
    }

    return m_nextViewportID++;
}
