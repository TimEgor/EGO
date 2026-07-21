#include "ApplicationGuiViewportSystem.h"

#include <cstdint>
#include <limits>
#include <utility>

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "EgoApplication/Application.h"
#include "EgoApplication/Window/WindowGraphicPresenter.h"

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

bool ego::application::ApplicationGuiViewportSystem::init(
    const ApplicationPointer& _application,
    const ApplicationWindowPointer& _primaryWindow,
    const GraphicPresenterPointer& _primaryGraphicPresenter)
{
    EGO_CHECK_INITIALIZATION(!m_application && !m_primaryWindow && !m_primaryGraphicPresenter && m_viewports.empty());
    EGO_CHECK_INITIALIZATION(_application);
    EGO_CHECK_INITIALIZATION(_primaryWindow && _primaryWindow->isValid());
    EGO_CHECK_INITIALIZATION(_primaryGraphicPresenter);

    m_application = _application;
    m_primaryWindow = _primaryWindow;
    m_primaryGraphicPresenter = _primaryGraphicPresenter;
    return true;
}

void ego::application::ApplicationGuiViewportSystem::release()
{
    for (ViewportMap::value_type& viewportEntry : m_viewports)
    {
        if (viewportEntry.second.m_guiHost)
        {
            viewportEntry.second.m_guiHost->beginClosing();
        }
    }

    m_viewports.clear();
    m_primaryGraphicPresenter = nullptr;
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

    GraphicPresenterPointer graphicPresenter = m_primaryGraphicPresenter;
    if (isSecondary)
    {
        const gpu::GraphicHardwareSubsystemPointer graphicHardwareSubsystem = gpu::GetGraphicHardwareSubsystemPointer();
        const GraphicDevicePointer graphicDevice = graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicDevicePointer() : nullptr;
        if (!graphicDevice)
        {
            window->release();
            return false;
        }

        WindowGraphicPresenterPointer windowGraphicPresenter = new WindowGraphicPresenter();
        if (!windowGraphicPresenter)
        {
            window->release();
            return false;
        }

        gpu::SwapChainDesc swapChainDesc;
        swapChainDesc.m_format = gpu::GraphicResourceFormat::R8G8B8A8UNorm;
        swapChainDesc.m_bufferCount = 2;
        if (!windowGraphicPresenter->init(*graphicDevice, *window, swapChainDesc, graphicHardwareSubsystem->getGraphicCommandQueue()))
        {
            window->release();
            return false;
        }

        graphicPresenter = windowGraphicPresenter;
    }

    ApplicationWindowGuiViewportHostPointer viewportHost = new ApplicationWindowGuiViewportHost();
    EGO_CHECK_RETURN_FALSE(viewportHost);

    if (!viewportHost->init(window))
    {
        return false;
    }

    ViewportEntry entry;
    entry.m_guiHost = viewportHost;
    entry.m_graphicPresenter = graphicPresenter;
    m_viewports.emplace(_request.m_id, std::move(entry));

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

    if (viewportIt->second.m_guiHost)
    {
        viewportIt->second.m_guiHost->beginClosing();
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
    const GraphicPresenterPointer graphicPresenter = findGraphicPresenter(_viewportID);
    update.m_graphicPresenter = graphicPresenter;
    if (update.m_status == gui::ViewportUpdateStatus::Alive && !graphicPresenter)
    {
        update.m_status = gui::ViewportUpdateStatus::Lost;
    }
    host->drainInput(update.m_input);

    return update;
}

ego::application::ApplicationWindowGuiViewportHostPointer ego::application::ApplicationGuiViewportSystem::findViewport(gui::ViewportID _viewportID) const
{
    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second.m_guiHost : nullptr;
}

ego::GraphicPresenterPointer ego::application::ApplicationGuiViewportSystem::findGraphicPresenter(gui::ViewportID _viewportID) const
{
    const ViewportMap::const_iterator viewportIt = m_viewports.find(_viewportID);
    return viewportIt != m_viewports.end() ? viewportIt->second.m_graphicPresenter : nullptr;
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
