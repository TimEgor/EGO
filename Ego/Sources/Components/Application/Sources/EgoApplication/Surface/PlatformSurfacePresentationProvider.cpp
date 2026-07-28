#include "PlatformSurfacePresentationProvider.h"

#include <cstdint>
#include <utility>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Event/EventSubsystem.h"
#include "EgoCore/Platform/Platform.h"
#include "EgoCore/Platform/PlatformSubsystem.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/ApplicationEvents.h"

ego::application::PlatformSurfacePresentationProvider::~PlatformSurfacePresentationProvider()
{
    release();
}

bool ego::application::PlatformSurfacePresentationProvider::init(const InitData& _initData)
{
    EGO_CHECK_INITIALIZATION(!m_isInitialized && m_presentations.empty() && !m_mainSurface);

    const PlatformPointer platform = GetPlatformPointer();
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_INITIALIZATION(platform && eventController);

    m_swapChainDesc = _initData.m_swapChainDesc;

    EGO_CHECK_RETURN_CALL_FALSE(registerApplicationEvents(), release());

    m_isInitialized = true;

    return true;
}

void ego::application::PlatformSurfacePresentationProvider::release()
{
    releasePresentations();
    unregisterApplicationEvents();

    m_swapChainDesc = gpu::SwapChainDesc();
    m_isInitialized = false;
}

ego::application::Presentation ego::application::PlatformSurfacePresentationProvider::createPresentation(const PresentationDesc& _desc)
{
    Presentation presentation;
    if (!m_isInitialized)
    {
        return presentation;
    }

    PlatformSurfacePointer surface = getSurfaceController().createSurface(_desc);
    if (!surface)
    {
        return presentation;
    }

    PresentationEntry entry;
    entry.m_surface = surface;
    if (!registerSurfaceEvents(entry))
    {
        getSurfaceController().destroySurface(surface);
        return presentation;
    }

    SurfaceGraphicPresenterPointer graphicPresenter = createGraphicPresenter(*surface);
    if (!graphicPresenter)
    {
        releasePresentation(entry, true);

        return presentation;
    }

    entry.m_graphicPresenter = graphicPresenter;

    if (!m_mainSurface)
    {
        m_mainSurface = surface;
    }

    presentation.m_surface = surface;
    presentation.m_graphicPresenter = graphicPresenter;

    m_presentations.push_back(std::move(entry));

    return presentation;
}

bool ego::application::PlatformSurfacePresentationProvider::destroyPresentation(const PlatformSurfacePointer& _surface)
{
    if (!_surface)
    {
        return false;
    }

    const PresentationCollection::iterator presentationIt = findPresentation(*_surface);
    if (presentationIt == m_presentations.end())
    {
        return false;
    }

    PresentationEntry presentation = std::move(*presentationIt);
    m_presentations.erase(presentationIt);

    if (m_mainSurface.get() == presentation.m_surface.get())
    {
        m_mainSurface = nullptr;
    }

    releasePresentation(presentation, true);

    return true;
}

ego::GraphicPresenterPointer ego::application::PlatformSurfacePresentationProvider::findGraphicPresenter(const PlatformSurfacePointer& _surface) const
{
    if (!_surface)
    {
        return nullptr;
    }

    const PresentationCollection::const_iterator presentationIt = findPresentation(*_surface);

    return presentationIt != m_presentations.end() ? presentationIt->m_graphicPresenter : nullptr;
}

void ego::application::PlatformSurfacePresentationProvider::processEvents()
{
    if (m_isInitialized)
    {
        getSurfaceController().processEvents();
    }
}

ego::PlatformSurfaceController& ego::application::PlatformSurfacePresentationProvider::getSurfaceController() const
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_ASSERT(platform);

    return platform->getSurfaceController();
}

bool ego::application::PlatformSurfacePresentationProvider::registerApplicationEvents()
{
    EGO_CHECK_RETURN_FALSE(!m_areApplicationEventsRegistered);

    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);
    EGO_CHECK_RETURN_FALSE(eventController->registerEvent<ApplicationQuitRequestedEvent>());

    m_areApplicationEventsRegistered = true;

    return true;
}

void ego::application::PlatformSurfacePresentationProvider::unregisterApplicationEvents()
{
    if (!m_areApplicationEventsRegistered)
    {
        return;
    }

    const EventControllerPointer eventController = GetEventControllerPointer();
    if (eventController)
    {
        eventController->unregisterEvent<ApplicationQuitRequestedEvent>();
    }

    m_areApplicationEventsRegistered = false;
}

bool ego::application::PlatformSurfacePresentationProvider::registerSurfaceEvents(PresentationEntry& _presentation)
{
    EGO_CHECK_RETURN_FALSE(_presentation.m_surface);

    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN_FALSE(eventController);

    const PlatformSurfaceEventIDs& eventIDs = _presentation.m_surface->getEventIDs();
    SurfaceEventCallbackIDs& callbackIDs = _presentation.m_eventCallbackIDs;

    callbackIDs.m_closeRequested = eventController->addInstanceEventCallback<PlatformSurfaceCloseRequestedEvent>(
        eventIDs.m_closeRequested,
        *this,
        &PlatformSurfacePresentationProvider::handleSurfaceCloseRequested);
    EGO_CHECK_RETURN_CALL_FALSE(callbackIDs.m_closeRequested != InvalidInstancedEventCallbackID, unregisterSurfaceEvents(_presentation));

    callbackIDs.m_sizeChanged = eventController->addInstanceEventCallback<PlatformSurfaceSizeChangedEvent>(
        eventIDs.m_sizeChanged,
        *this,
        &PlatformSurfacePresentationProvider::handleSurfaceSizeChanged);
    EGO_CHECK_RETURN_CALL_FALSE(callbackIDs.m_sizeChanged != InvalidInstancedEventCallbackID, unregisterSurfaceEvents(_presentation));

    return true;
}

void ego::application::PlatformSurfacePresentationProvider::unregisterSurfaceEvents(PresentationEntry& _presentation)
{
    if (!_presentation.m_surface)
    {
        _presentation.m_eventCallbackIDs = SurfaceEventCallbackIDs();
        return;
    }

    const EventControllerPointer eventController = GetEventControllerPointer();
    SurfaceEventCallbackIDs& callbackIDs = _presentation.m_eventCallbackIDs;
    if (eventController)
    {
        eventController->removeInstancedEventDispatcher(callbackIDs.m_sizeChanged);
        eventController->removeInstancedEventDispatcher(callbackIDs.m_closeRequested);
    }

    callbackIDs = SurfaceEventCallbackIDs();
}

void ego::application::PlatformSurfacePresentationProvider::releasePresentations()
{
    while (!m_presentations.empty())
    {
        PresentationEntry presentation = std::move(m_presentations.back());
        m_presentations.pop_back();
        releasePresentation(presentation, true);
    }

    m_mainSurface = nullptr;
}

void ego::application::PlatformSurfacePresentationProvider::releasePresentation(PresentationEntry& _presentation, bool _destroySurface)
{
    unregisterSurfaceEvents(_presentation);

    EGO_SAFE_RESET_POINTER_WITH_RELEASING(_presentation.m_graphicPresenter);

    if (_destroySurface)
    {
        getSurfaceController().destroySurface(_presentation.m_surface);
        _presentation.m_surface = nullptr;
    }
    else
    {
        _presentation.m_surface = nullptr;
    }
}

ego::application::PlatformSurfacePresentationProvider::PresentationCollection::iterator ego::application::PlatformSurfacePresentationProvider::findPresentation(
    PlatformSurface& _surface)
{
    for (PresentationCollection::iterator presentationIt = m_presentations.begin(); presentationIt != m_presentations.end(); ++presentationIt)
    {
        if (presentationIt->m_surface.get() == &_surface)
        {
            return presentationIt;
        }
    }

    return m_presentations.end();
}

ego::application::PlatformSurfacePresentationProvider::PresentationCollection::const_iterator ego::application::PlatformSurfacePresentationProvider::
    findPresentation(const PlatformSurface& _surface) const
{
    for (PresentationCollection::const_iterator presentationIt = m_presentations.begin(); presentationIt != m_presentations.end(); ++presentationIt)
    {
        if (presentationIt->m_surface.get() == &_surface)
        {
            return presentationIt;
        }
    }

    return m_presentations.end();
}

ego::application::SurfaceGraphicPresenterPointer ego::application::PlatformSurfacePresentationProvider::createGraphicPresenter(
    const PlatformSurface& _surface) const
{
    const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
    const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicDevicePointer() : nullptr;
    const gpu::CommandQueuePointer presentationQueue = graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicCommandQueue() : nullptr;
    EGO_CHECK_RETURN_NULL(graphicDevice && presentationQueue);

    SurfaceGraphicPresenterPointer graphicPresenter =
        MakePointer<SurfaceGraphicPresenter>();
    if (!graphicPresenter || !graphicPresenter->init(*graphicDevice, _surface, m_swapChainDesc, presentationQueue))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(graphicPresenter);

        return nullptr;
    }

    return graphicPresenter;
}

void ego::application::PlatformSurfacePresentationProvider::handleSurfaceCloseRequested(const PlatformSurfaceCloseRequestedEvent& _event)
{
    if (m_mainSurface.get() == &_event.m_surface)
    {
        emitApplicationQuitRequested();
    }

    _event.handle();
}

void ego::application::PlatformSurfacePresentationProvider::handleSurfaceSizeChanged(const PlatformSurfaceSizeChangedEvent& _event)
{
    const PresentationCollection::iterator presentationIt = findPresentation(_event.m_surface);
    if (presentationIt == m_presentations.end() || !presentationIt->m_graphicPresenter)
    {
        return;
    }

    const SurfaceSize& surfaceSize = _event.m_surface.getSize();
    if (surfaceSize.m_x == 0 || surfaceSize.m_y == 0)
    {
        return;
    }

    const gpu::Texture2DSize targetSize(static_cast<uint32_t>(surfaceSize.m_x), static_cast<uint32_t>(surfaceSize.m_y));
    const bool resizeResult = presentationIt->m_graphicPresenter->resize(targetSize);
    EGO_ASSERT_MESSAGE(resizeResult, "Failed to request a window graphic presenter resize.");
}

ego::EventControllerPointer ego::application::PlatformSurfacePresentationProvider::GetEventControllerPointer()
{
    const EventSubsystemPointer eventSubsystem = GetEventSubsystemPointer();

    return eventSubsystem ? eventSubsystem->getEventControllerPointer() : nullptr;
}

void ego::application::PlatformSurfacePresentationProvider::emitApplicationQuitRequested() const
{
    const EventControllerPointer eventController = GetEventControllerPointer();
    EGO_CHECK_RETURN(eventController);

    const ApplicationQuitRequestedEvent event;
    eventController->emitEvent(event);
}
