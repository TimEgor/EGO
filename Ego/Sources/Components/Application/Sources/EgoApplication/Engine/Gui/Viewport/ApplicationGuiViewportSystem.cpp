#include "ApplicationGuiViewportSystem.h"

#include <cstdint>
#include <limits>

#include "EgoCore/UtilsMacros.h"

#include "EgoApplication/Application.h"

#include "Window/ApplicationWindowGuiViewportHost.h"

namespace
{
    uint16_t ToWindowSizeValue(float _value)
    {
        if (_value <= 0.0f)
        {
            return 0;
        }

        const float maxValue = static_cast<float>((std::numeric_limits<uint16_t>::max)());
        return static_cast<uint16_t>(_value < maxValue ? _value : maxValue);
    }

} // namespace

ego::application::ApplicationGuiViewportSystem::~ApplicationGuiViewportSystem()
{
    release();
}

bool ego::application::ApplicationGuiViewportSystem::init(const ApplicationPointer& _application, const ApplicationWindowPointer& _primaryWindow)
{
    EGO_CHECK_INITIALIZATION(!m_application && !m_primaryWindow && m_viewports.empty());
    EGO_CHECK_INITIALIZATION(_application);
    EGO_CHECK_INITIALIZATION(_primaryWindow && _primaryWindow->isValid());

    m_application = _application;
    m_primaryWindow = _primaryWindow;
    return true;
}

void ego::application::ApplicationGuiViewportSystem::release()
{
    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        if (viewportEntry.second)
        {
            viewportEntry.second->beginClosing();
        }
    }

    m_viewports.clear();
    m_primaryWindow = nullptr;
    m_application = nullptr;
}

bool ego::application::ApplicationGuiViewportSystem::createViewport(const gui::ViewportCreateRequest& _request)
{
    EGO_CHECK_RETURN_FALSE(_request.m_id != gui::InvalidViewportID);
    EGO_CHECK_RETURN_FALSE(m_application && m_primaryWindow);

    if (m_viewports.contains(_request.m_id))
    {
        return false;
    }

    ApplicationWindowGuiViewportHostPointer host = new ApplicationWindowGuiViewportHost();
    EGO_CHECK_RETURN_FALSE(host);

    const bool isSecondary = _request.m_role == gui::ViewportRole::Secondary;
    const ApplicationWindowPointer window = isSecondary ? m_application->createWindow(CreateSecondaryWindowDesc(_request)) : m_primaryWindow;
    if (!window || !window->isValid())
    {
        if (window && isSecondary)
        {
            window->release();
        }
        return false;
    }

    if (!host->init(window, _request.m_role))
    {
        return false;
    }

    m_viewports.emplace(_request.m_id, host);

    if (isSecondary)
    {
        window->show();
    }

    return true;
}

void ego::application::ApplicationGuiViewportSystem::destroyViewport(gui::ViewportID _viewportID)
{
    if (_viewportID == gui::InvalidViewportID)
    {
        return;
    }

    ViewportMap::iterator viewportIt = m_viewports.find(_viewportID);
    if (viewportIt == m_viewports.end())
    {
        return;
    }

    if (viewportIt->second)
    {
        viewportIt->second->beginClosing();
    }

    m_viewports.erase(viewportIt);
}

ego::gui::ViewportUpdate ego::application::ApplicationGuiViewportSystem::pollViewport(gui::ViewportID _viewportID)
{
    gui::ViewportUpdate update;

    const ApplicationWindowGuiViewportHostPointer host = findViewport(_viewportID);
    if (!host)
    {
        return update;
    }

    host->update();
    update.m_size = host->getSize();
    update.m_status = host->getUpdateStatus();
    host->drainInput(update.m_input);

    return update;
}

ego::engine::EngineViewportHostPointer ego::application::ApplicationGuiViewportSystem::findViewportHost(gui::ViewportID _viewportID) const
{
    const ApplicationWindowGuiViewportHostPointer host = findViewport(_viewportID);
    if (!host)
    {
        return nullptr;
    }

    return host->getUpdateStatus() == gui::ViewportUpdateStatus::Alive ? host : nullptr;
}

ego::application::ApplicationWindowGuiViewportHostPointer ego::application::ApplicationGuiViewportSystem::findViewport(gui::ViewportID _viewportID) const
{
    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second : nullptr;
}

ego::WindowDesc ego::application::ApplicationGuiViewportSystem::CreateSecondaryWindowDesc(const gui::ViewportCreateRequest& _request)
{
    constexpr WindowSize defaultViewportSize(500, 500);

    WindowDesc windowDesc;
    windowDesc.m_title = "EGO Viewport";
    windowDesc.m_showOnInit = false;
    windowDesc.m_size = WindowSize(ToWindowSizeValue(_request.m_desc.m_size.m_x), ToWindowSizeValue(_request.m_desc.m_size.m_y));
    if (windowDesc.m_size.m_x == 0 || windowDesc.m_size.m_y == 0)
    {
        windowDesc.m_size = defaultViewportSize;
    }

    return windowDesc;
}
