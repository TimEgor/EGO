#include "ImGuiPlatformAdapter.h"

#include <cstddef>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/UtilsMacros.h"

#include <imgui.h>

namespace
{
    constexpr float VirtualDesktopMinimum = -1048576.0f;
    constexpr float VirtualDesktopSize = 2097152.0f;
    constexpr const char* BackendName = "EgoGui_ImGuiPlatform";
    constexpr const char* SecondaryViewportTitle = "EGO ImGui Viewport";

} // namespace

struct ego::gui::ImGuiPlatformAdapter::ViewportData final
{
    ViewportID m_viewportID = InvalidViewportID;
};

struct ego::gui::ImGuiPlatformAdapter::ViewportPollResult final
{
    bool m_isAlive = false;
    bool m_isFocused = false;
};

ego::gui::ImGuiPlatformAdapter::~ImGuiPlatformAdapter()
{
    EGO_ASSERT(!m_provider);
}

bool ego::gui::ImGuiPlatformAdapter::init(const ViewportProviderPointer& _provider)
{
    EGO_CHECK_INITIALIZATION(!m_provider && !m_context);
    EGO_CHECK_RETURN_FALSE(_provider);

    ImGuiContext* context = ImGui::GetCurrentContext();
    EGO_CHECK_RETURN_FALSE(context);

    m_provider = _provider;
    m_context = context;

    ViewportCreateRequest request;
    request.m_id = PrimaryViewportID;
    request.m_role = ViewportRole::Primary;
    EGO_CHECK_INITIALIZATION(m_provider->createViewport(request));

    m_primaryViewportID = PrimaryViewportID;

    const ViewportState primaryState = m_provider->getViewportState(m_primaryViewportID);
    EGO_CHECK_INITIALIZATION(primaryState.m_status == ViewportUpdateStatus::Alive && primaryState.m_graphicPresenter);

    ImGuiViewport* primaryViewport = ImGui::GetMainViewport();
    EGO_CHECK_INITIALIZATION(primaryViewport && !primaryViewport->PlatformUserData && !primaryViewport->PlatformHandle);

    ViewportDataOwner primaryData = std::make_unique<ViewportData>();
    EGO_CHECK_INITIALIZATION(primaryData);

    primaryData->m_viewportID = m_primaryViewportID;

    m_nextViewportID = PrimaryViewportID + 1;

    primaryViewport->PlatformUserData = primaryData.release();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = BackendName;
    io.BackendPlatformUserData = this;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_CreateWindow = PlatformCreateWindow;
    platformIO.Platform_DestroyWindow = PlatformDestroyWindow;
    platformIO.Platform_ShowWindow = PlatformShowWindow;
    platformIO.Platform_SetWindowPos = PlatformSetWindowPosition;
    platformIO.Platform_GetWindowPos = PlatformGetWindowPosition;
    platformIO.Platform_SetWindowSize = PlatformSetWindowSize;
    platformIO.Platform_GetWindowSize = PlatformGetWindowSize;
    platformIO.Platform_GetWindowFramebufferScale = PlatformGetWindowFramebufferScale;
    platformIO.Platform_SetWindowFocus = PlatformSetWindowFocus;
    platformIO.Platform_GetWindowFocus = PlatformGetWindowFocus;
    platformIO.Platform_SetWindowTitle = PlatformSetWindowTitle;
    platformIO.Platform_SetWindowAlpha = PlatformSetWindowAlpha;
    platformIO.Platform_UpdateWindow = PlatformUpdateWindow;

    ImGuiPlatformMonitor monitor;
    monitor.MainPos = ImVec2(VirtualDesktopMinimum, VirtualDesktopMinimum);
    monitor.MainSize = ImVec2(VirtualDesktopSize, VirtualDesktopSize);
    monitor.WorkPos = monitor.MainPos;
    monitor.WorkSize = monitor.MainSize;
    monitor.DpiScale = 1.0f;
    platformIO.Monitors.push_back(monitor);

    updatePrimaryDisplay();

    return true;
}

void ego::gui::ImGuiPlatformAdapter::release()
{
    if (!m_provider)
    {
        return;
    }

    EGO_ASSERT(ImGui::GetCurrentContext() == m_context);
    if (ImGui::GetCurrentContext() != m_context)
    {
        return;
    }

    ImGui::DestroyPlatformWindows();

    ImGuiViewport* primaryViewport = ImGui::GetMainViewport();
    if (primaryViewport)
    {
        destroyViewport(*primaryViewport);
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendPlatformUserData == this)
    {
        io.BackendPlatformName = nullptr;
        io.BackendPlatformUserData = nullptr;
        io.BackendFlags &= ~ImGuiBackendFlags_PlatformHasViewports;
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    }

    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    platformIO.ClearPlatformHandlers();
    platformIO.Monitors.clear();

    if (m_primaryViewportID != InvalidViewportID)
    {
        m_provider->destroyViewport(m_primaryViewportID);
    }

    m_input.reset();
    m_provider = nullptr;
    m_context = nullptr;
    m_primaryViewportID = InvalidViewportID;
    m_nextViewportID = InvalidViewportID;
    m_hasFatalPlatformError = false;
}

bool ego::gui::ImGuiPlatformAdapter::beginFrame()
{
    EGO_CHECK_RETURN_FALSE(m_provider && ImGui::GetCurrentContext() == m_context && !m_hasFatalPlatformError);

    bool isPrimaryAlive = false;
    bool isAnyViewportFocused = false;
    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    for (ImGuiViewport* imguiViewport : platformIO.Viewports)
    {
        const ViewportData* data = GetViewportData(imguiViewport);
        if (!data)
        {
            continue;
        }

        const ViewportPollResult result = pollViewport(*imguiViewport, *data);
        isAnyViewportFocused |= result.m_isFocused;
        if (data->m_viewportID == m_primaryViewportID)
        {
            isPrimaryAlive = result.m_isAlive;
        }
    }

    ImGui::GetIO().AddFocusEvent(isAnyViewportFocused);
    updatePrimaryDisplay();

    return isPrimaryAlive;
}

bool ego::gui::ImGuiPlatformAdapter::endFrame()
{
    EGO_CHECK_RETURN_FALSE(m_provider && ImGui::GetCurrentContext() == m_context && !m_hasFatalPlatformError);

    ImGui::UpdatePlatformWindows();

    return !m_hasFatalPlatformError;
}

ego::gui::ImGuiPlatformRenderTargetCollection ego::gui::ImGuiPlatformAdapter::getRenderTargets() const
{
    ImGuiPlatformRenderTargetCollection targets;
    EGO_CHECK_RETURN_VALUE(m_provider && ImGui::GetCurrentContext() == m_context, targets);

    const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    targets.reserve(static_cast<size_t>(platformIO.Viewports.Size));
    for (ImGuiViewport* imguiViewport : platformIO.Viewports)
    {
        const ViewportData* data = GetViewportData(imguiViewport);
        if (!data)
        {
            continue;
        }

        const ViewportState state = m_provider->getViewportState(data->m_viewportID);
        if (state.m_status != ViewportUpdateStatus::Alive || !state.m_graphicPresenter || !imguiViewport->DrawData ||
            (imguiViewport->Flags & ImGuiViewportFlags_IsMinimized) != 0)
        {
            continue;
        }

        targets.push_back(
            {
                .m_drawData = imguiViewport->DrawData,
                .m_graphicPresenter = state.m_graphicPresenter,
            });
    }

    return targets;
}

ego::gui::ImGuiPlatformAdapter::ViewportPollResult ego::gui::ImGuiPlatformAdapter::pollViewport(ImGuiViewport& _viewport, const ViewportData& _data)
{
    ViewportPollResult result;
    EGO_CHECK_RETURN_VALUE(m_provider, result);

    const ViewportUpdate update = m_provider->pollViewport(_data.m_viewportID);
    result.m_isAlive = update.m_state.m_status == ViewportUpdateStatus::Alive;
    result.m_isFocused = result.m_isAlive && update.m_state.m_isFocused;
    if (!result.m_isAlive)
    {
        _viewport.PlatformRequestClose = true;

        return result;
    }

    if (update.m_positionChanged)
    {
        _viewport.PlatformRequestMove = true;
    }

    if (update.m_sizeChanged)
    {
        _viewport.PlatformRequestResize = true;
    }

    for (const InputEventOwner& inputEvent : update.m_input)
    {
        if (inputEvent)
        {
            inputEvent->dispatch(m_input);
        }
    }

    return result;
}

void ego::gui::ImGuiPlatformAdapter::destroyViewport(ImGuiViewport& _viewport)
{
    ViewportDataOwner data = TakeViewportData(_viewport);
    if (!data)
    {
        return;
    }

    if (data->m_viewportID != m_primaryViewportID && m_provider)
    {
        m_provider->destroyViewport(data->m_viewportID);
    }
}

void ego::gui::ImGuiPlatformAdapter::updatePrimaryDisplay()
{
    const ViewportData* primaryData = GetViewportData(ImGui::GetMainViewport());
    if (!primaryData || primaryData->m_viewportID != m_primaryViewportID)
    {
        return;
    }

    const ViewportState state = m_provider->getViewportState(primaryData->m_viewportID);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(state.m_size.m_x, state.m_size.m_y);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
}

void ego::gui::ImGuiPlatformAdapter::markViewportFailure(ImGuiViewport& _viewport)
{
    m_hasFatalPlatformError = true;
    _viewport.PlatformRequestClose = true;
}

ego::gui::ViewportID ego::gui::ImGuiPlatformAdapter::allocateViewportID()
{
    const ViewportID firstCandidate = m_nextViewportID;
    do
    {
        const ViewportID candidate = m_nextViewportID;
        ++m_nextViewportID;
        if (m_nextViewportID == InvalidViewportID)
        {
            m_nextViewportID = 1;
        }

        bool isAllocated = false;
        const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
        for (const ImGuiViewport* viewport : platformIO.Viewports)
        {
            const ViewportData* data = GetViewportData(viewport);
            if (data && data->m_viewportID == candidate)
            {
                isAllocated = true;
                break;
            }
        }

        if (candidate != InvalidViewportID && !isAllocated)
        {
            return candidate;
        }
    } while (m_nextViewportID != firstCandidate);

    return InvalidViewportID;
}

ego::gui::ImGuiPlatformAdapter* ego::gui::ImGuiPlatformAdapter::GetCurrentAdapter()
{
    ImGuiContext* context = ImGui::GetCurrentContext();
    if (!context)
    {
        return nullptr;
    }

    return static_cast<ImGuiPlatformAdapter*>(ImGui::GetIO().BackendPlatformUserData);
}

ego::gui::ImGuiPlatformAdapter::ViewportData* ego::gui::ImGuiPlatformAdapter::GetViewportData(ImGuiViewport* _viewport)
{
    if (!_viewport || !_viewport->PlatformUserData)
    {
        return nullptr;
    }

    return static_cast<ViewportData*>(_viewport->PlatformUserData);
}

const ego::gui::ImGuiPlatformAdapter::ViewportData* ego::gui::ImGuiPlatformAdapter::GetViewportData(const ImGuiViewport* _viewport)
{
    if (!_viewport || !_viewport->PlatformUserData)
    {
        return nullptr;
    }

    return static_cast<const ViewportData*>(_viewport->PlatformUserData);
}

ego::gui::ImGuiPlatformAdapter::ViewportDataOwner ego::gui::ImGuiPlatformAdapter::TakeViewportData(ImGuiViewport& _viewport)
{
    ViewportDataOwner data(static_cast<ViewportData*>(_viewport.PlatformUserData));
    _viewport.PlatformUserData = nullptr;

    return data;
}

void ego::gui::ImGuiPlatformAdapter::PlatformCreateWindow(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    if (!adapter || !_viewport || _viewport->PlatformUserData)
    {
        return;
    }

    const ViewportID viewportID = adapter->allocateViewportID();
    if (viewportID == InvalidViewportID)
    {
        adapter->markViewportFailure(*_viewport);

        return;
    }

    ViewportDataOwner data = std::make_unique<ViewportData>();
    data->m_viewportID = viewportID;

    ViewportCreateRequest request;
    request.m_id = viewportID;
    request.m_role = ViewportRole::Secondary;
    request.m_title = SecondaryViewportTitle;
    request.m_position = FloatVector2(_viewport->Pos.x, _viewport->Pos.y);
    request.m_size = FloatVector2(_viewport->Size.x, _viewport->Size.y);
    if (!adapter->m_provider->createViewport(request))
    {
        adapter->markViewportFailure(*_viewport);

        return;
    }

    const ViewportState state = adapter->m_provider->getViewportState(viewportID);
    _viewport->PlatformUserData = data.release();
    if (state.m_status != ViewportUpdateStatus::Alive || !state.m_graphicPresenter)
    {
        adapter->markViewportFailure(*_viewport);
    }
}

void ego::gui::ImGuiPlatformAdapter::PlatformDestroyWindow(ImGuiViewport* _viewport)
{
    if (!_viewport)
    {
        return;
    }

    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    if (!adapter)
    {
        TakeViewportData(*_viewport);

        return;
    }

    adapter->destroyViewport(*_viewport);
}

void ego::gui::ImGuiPlatformAdapter::PlatformShowWindow(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data || data->m_viewportID == adapter->m_primaryViewportID)
    {
        return;
    }

    const bool activate = (_viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing) == 0;
    adapter->m_provider->showViewport(data->m_viewportID, activate);
}

void ego::gui::ImGuiPlatformAdapter::PlatformSetWindowPosition(ImGuiViewport* _viewport, ImVec2 _position)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data || data->m_viewportID == adapter->m_primaryViewportID)
    {
        return;
    }

    FloatVector2 position(_position.x, _position.y);
    if (adapter->m_provider->setViewportPosition(data->m_viewportID, position))
    {
        _viewport->Pos = ImVec2(position.m_x, position.m_y);
    }
}

ImVec2 ego::gui::ImGuiPlatformAdapter::PlatformGetWindowPosition(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    const ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data)
    {
        return ImVec2();
    }

    const ViewportState state = adapter->m_provider->getViewportState(data->m_viewportID);
    return ImVec2(state.m_position.m_x, state.m_position.m_y);
}

void ego::gui::ImGuiPlatformAdapter::PlatformSetWindowSize(ImGuiViewport* _viewport, ImVec2 _size)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data || data->m_viewportID == adapter->m_primaryViewportID)
    {
        return;
    }

    FloatVector2 size(_size.x, _size.y);
    if (adapter->m_provider->setViewportSize(data->m_viewportID, size))
    {
        _viewport->Size = ImVec2(size.m_x, size.m_y);
    }
}

ImVec2 ego::gui::ImGuiPlatformAdapter::PlatformGetWindowSize(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    const ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data)
    {
        return ImVec2();
    }

    const ViewportState state = adapter->m_provider->getViewportState(data->m_viewportID);
    return ImVec2(state.m_size.m_x, state.m_size.m_y);
}

ImVec2 ego::gui::ImGuiPlatformAdapter::PlatformGetWindowFramebufferScale(ImGuiViewport*)
{
    return ImVec2(1.0f, 1.0f);
}

void ego::gui::ImGuiPlatformAdapter::PlatformSetWindowFocus(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data || data->m_viewportID == adapter->m_primaryViewportID)
    {
        return;
    }

    adapter->m_provider->showViewport(data->m_viewportID, true);
}

bool ego::gui::ImGuiPlatformAdapter::PlatformGetWindowFocus(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    const ViewportData* data = GetViewportData(_viewport);
    return adapter && data && adapter->m_provider->getViewportState(data->m_viewportID).m_isFocused;
}

void ego::gui::ImGuiPlatformAdapter::PlatformSetWindowTitle(ImGuiViewport*, const char*) {}

void ego::gui::ImGuiPlatformAdapter::PlatformSetWindowAlpha(ImGuiViewport*, float) {}

void ego::gui::ImGuiPlatformAdapter::PlatformUpdateWindow(ImGuiViewport* _viewport)
{
    ImGuiPlatformAdapter* adapter = GetCurrentAdapter();
    ViewportData* data = GetViewportData(_viewport);
    if (!adapter || !data || data->m_viewportID == adapter->m_primaryViewportID)
    {
        return;
    }

    const bool isInputPassthroughEnabled = (_viewport->Flags & ImGuiViewportFlags_NoInputs) != 0;
    const ViewportState state = adapter->m_provider->getViewportState(data->m_viewportID);
    if (state.m_isInputPassthroughEnabled == isInputPassthroughEnabled)
    {
        return;
    }

    adapter->m_provider->setViewportInputPassthrough(data->m_viewportID, isInputPassthroughEnabled);
}
