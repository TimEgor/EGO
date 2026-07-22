#include "ApplicationGuiViewportProvider.h"

#include <limits>

#include "EgoCore/UtilsMacros.h"

#include "EgoApplication/ApplicationSubsystem.h"

#include "ApplicationGuiViewport.h"

namespace
{
    uint16_t ToPresentationSurfaceSizeValue(float _value)
    {
        if (_value <= 0.0f)
        {
            return 0;
        }

        const float maxValue = static_cast<float>((std::numeric_limits<uint16_t>::max)());
        return static_cast<uint16_t>(_value < maxValue ? _value : maxValue);
    }
} // namespace

ego::application::ApplicationGuiViewportProvider::~ApplicationGuiViewportProvider()
{
    release();
}

bool ego::application::ApplicationGuiViewportProvider::init(const Presentation& _primaryPresentation)
{
    EGO_CHECK_INITIALIZATION(!m_primaryPresentation.m_surface && m_viewports.empty());
    EGO_CHECK_INITIALIZATION(_primaryPresentation.m_surface && _primaryPresentation.m_graphicPresenter);

    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    EGO_CHECK_INITIALIZATION(presenterProvider);
    EGO_CHECK_INITIALIZATION(
        presenterProvider->findGraphicPresenter(_primaryPresentation.m_surface).get() ==
        _primaryPresentation.m_graphicPresenter.get());

    m_primaryPresentation = _primaryPresentation;
    return true;
}

void ego::application::ApplicationGuiViewportProvider::release()
{
    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        releaseViewport(viewportEntry.second);
    }

    m_viewports.clear();
    m_primaryPresentation = Presentation();
}

bool ego::application::ApplicationGuiViewportProvider::createViewport(const gui::ViewportCreateRequest& _request)
{
    EGO_CHECK_RETURN_FALSE(_request.m_id != gui::InvalidViewportID);
    EGO_CHECK_RETURN_FALSE(m_primaryPresentation.m_surface);
    EGO_CHECK_RETURN_FALSE(!m_viewports.contains(_request.m_id));

    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    EGO_CHECK_RETURN_FALSE(presenterProvider);

    const bool isSecondary = _request.m_role == gui::ViewportRole::Secondary;
    const Presentation presentation = isSecondary ?
                                          presenterProvider->createPresentation(CreateViewportPresentationDesc(_request)) :
                                          m_primaryPresentation;
    if (!presentation.m_surface || !presentation.m_graphicPresenter)
    {
        if (isSecondary && presentation.m_surface)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }
        return false;
    }

    if (presenterProvider->findGraphicPresenter(presentation.m_surface).get() != presentation.m_graphicPresenter.get())
    {
        if (isSecondary)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }
        return false;
    }

    ViewportPointer viewport = new ApplicationGuiViewport();
    if (!viewport || !viewport->init(presentation))
    {
        EGO_SAFE_RESET_POINTER_WITH_RELEASING(viewport);
        if (isSecondary)
        {
            presenterProvider->destroyPresentation(presentation.m_surface);
        }
        return false;
    }

    const std::pair<ViewportMap::iterator, bool> insertResult = m_viewports.emplace(_request.m_id, viewport);
    if (!insertResult.second)
    {
        releaseViewport(viewport);
        return false;
    }

    if (isSecondary)
    {
        presentation.m_surface->show();
    }

    return true;
}

void ego::application::ApplicationGuiViewportProvider::destroyViewport(gui::ViewportID _viewportID)
{
    ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end())
    {
        return;
    }

    releaseViewport(viewportIt->second);
    m_viewports.erase(viewportIt);
}

ego::gui::ViewportUpdate ego::application::ApplicationGuiViewportProvider::pollViewport(gui::ViewportID _viewportID)
{
    const ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end() || !viewportIt->second)
    {
        return gui::ViewportUpdate();
    }

    const ViewportPointer viewport = viewportIt->second;
    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    const GraphicPresenterPointer graphicPresenter =
        presenterProvider ? presenterProvider->findGraphicPresenter(viewport->getSurfacePointer()) : nullptr;
    return viewport->poll(graphicPresenter);
}

void ego::application::ApplicationGuiViewportProvider::releaseViewport(ViewportPointer& _viewport)
{
    if (!_viewport)
    {
        return;
    }

    const PresentationSurfacePointer surface = _viewport->getSurfacePointer();
    _viewport->release();
    _viewport = nullptr;

    if (!surface || surface.get() == m_primaryPresentation.m_surface.get())
    {
        return;
    }

    const PresenterProviderPointer presenterProvider = GetPresenterProvider();
    if (presenterProvider)
    {
        presenterProvider->destroyPresentation(surface);
    }
}

ego::application::PresenterProviderPointer ego::application::ApplicationGuiViewportProvider::GetPresenterProvider()
{
    const ApplicationPointer application = GetApplicationPointer();
    return application ? application->getPresenterProviderPointer() : nullptr;
}

ego::application::PresentationDesc ego::application::ApplicationGuiViewportProvider::CreateViewportPresentationDesc(
    const gui::ViewportCreateRequest& _request)
{
    constexpr PresentationSurfaceSize defaultViewportSize(500, 500);

    PresentationDesc presentationDesc;
    presentationDesc.m_name = "EGO Viewport";
    presentationDesc.m_size = PresentationSurfaceSize(
        ToPresentationSurfaceSizeValue(_request.m_desc.m_size.m_x),
        ToPresentationSurfaceSizeValue(_request.m_desc.m_size.m_y));
    if (presentationDesc.m_size.m_x == 0 || presentationDesc.m_size.m_y == 0)
    {
        presentationDesc.m_size = defaultViewportSize;
    }

    return presentationDesc;
}
