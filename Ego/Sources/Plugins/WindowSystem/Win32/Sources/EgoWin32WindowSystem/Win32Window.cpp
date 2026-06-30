#include "Win32Window.h"

#include "EgoCore/UtilsMacros.h"

#include "Win32WindowSystem.h"

ego::win32::Win32Window::Win32Window(HINSTANCE _instance)
    : m_instance(_instance)
{
}

ego::win32::Win32Window::~Win32Window()
{
    release();
}

bool ego::win32::Win32Window::init(const WindowDesc& _desc)
{
    EGO_CHECK_INITIALIZATION(initWindowInstancedEvents());

    const WindowSize requestedWindowSize = _desc.m_size;
    HWND handle = CreateWindow(
        EGO_WIN32_WINDOW_SYSTEM_WND_CLASS_NAME,
        _desc.m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        requestedWindowSize.m_x,
        requestedWindowSize.m_y,
        nullptr,
        nullptr,
        m_instance,
        nullptr);
    EGO_CHECK_INITIALIZATION(handle);

    {
        std::scoped_lock locker(m_mutex);

        m_windowSize = requestedWindowSize;
        m_handle = handle;
    }

    auto windowData = new Win32WindowWeakPointer(weakFromThis());
    SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowData));

    ShowCursor(true);

    if (_desc.m_showOnInit)
    {
        show();
        UpdateWindow(handle);
    }

    updateSizes();

    return true;
}

void ego::win32::Win32Window::release()
{
    releaseWindowInstancedEvents();

    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    if (handle)
    {
        DestroyWindow(handle);
    }
}

bool ego::win32::Win32Window::isValid() const
{
    std::shared_lock locker(m_mutex);

    return m_handle;
}

void ego::win32::Win32Window::show()
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    EGO_ASSERT(handle);
    EGO_CHECK_RETURN(handle);

    ShowWindow(handle, SW_NORMAL);

    {
        std::scoped_lock locker(m_mutex);
        if (m_handle == handle)
        {
            m_isShown = true;
        }
    }
}

void ego::win32::Win32Window::hide()
{
    HWND handle = nullptr;
    {
        std::shared_lock locker(m_mutex);
        handle = m_handle;
    }

    EGO_ASSERT(handle);
    EGO_CHECK_RETURN(handle);

    ShowWindow(handle, SW_HIDE);

    {
        std::scoped_lock locker(m_mutex);
        if (m_handle == handle)
        {
            m_isShown = false;
        }
    }
}

bool ego::win32::Win32Window::isShown() const
{
    std::shared_lock locker(m_mutex);

    return m_handle && m_isShown;
}

void* ego::win32::Win32Window::getNativeHandle() const
{
    return m_handle;
}

const ego::WindowSize& ego::win32::Win32Window::getWindowSize() const
{
    std::shared_lock locker(m_mutex);

    return m_windowSize;
}

const ego::WindowSize& ego::win32::Win32Window::getClientAreaSize() const
{
    std::shared_lock locker(m_mutex);

    return m_clientAreaSize;
}

const ego::WindowArea& ego::win32::Win32Window::getCutoutsArea() const
{
    std::shared_lock locker(m_mutex);

    return m_cutoutsArea;
}

HWND ego::win32::Win32Window::getHandle() const
{
    return m_handle;
}

bool ego::win32::Win32Window::isStable() const
{
    std::shared_lock locker(m_mutex);

    return m_handle && m_isSizeStable;
}

void ego::win32::Win32Window::updateSizes()
{
    std::scoped_lock locker(m_mutex);

    RECT windowRect;
    GetWindowRect(m_handle, &windowRect);
    m_windowSize.m_x = static_cast<uint16_t>(windowRect.right - windowRect.left);
    m_windowSize.m_y = static_cast<uint16_t>(windowRect.bottom - windowRect.top);

    RECT clientSize;
    GetClientRect(m_handle, &clientSize);
    m_clientAreaSize.m_x = static_cast<uint16_t>(clientSize.right - clientSize.left);
    m_clientAreaSize.m_y = static_cast<uint16_t>(clientSize.bottom - clientSize.top);

    m_cutoutsArea.m_top = static_cast<WindowArea::ValueType>(clientSize.top);
    m_cutoutsArea.m_bottom = static_cast<WindowArea::ValueType>(m_windowSize.m_y - clientSize.bottom);
    m_cutoutsArea.m_right = static_cast<WindowArea::ValueType>(m_windowSize.m_x - clientSize.right);
    m_cutoutsArea.m_left = static_cast<WindowArea::ValueType>(clientSize.left);
}

void ego::win32::Win32Window::invalidate()
{
    std::scoped_lock locker(m_mutex);

    auto windowData = reinterpret_cast<Win32WindowWeakPointer*>(GetWindowLongPtr(m_handle, GWLP_USERDATA));
    EGO_SAFE_DESTROY(windowData);

    SetWindowLongPtr(m_handle, GWLP_USERDATA, 0);

    m_handle = nullptr;
    m_isShown = false;
}

void ego::win32::Win32Window::setSizeStabilization(bool _state)
{
    std::scoped_lock locker(m_mutex);

    m_isSizeStable = _state;
}

void ego::win32::Win32Window::Accessor::OnWindowDestroying(Win32Window& _window)
{
    _window.invalidate();
}

void ego::win32::Win32Window::Accessor::OnWindowTransformationStart(Win32Window& _window)
{
    _window.setSizeStabilization(false);
}

void ego::win32::Win32Window::Accessor::OnWindowTransformationEnd(Win32Window& _window)
{
    _window.setSizeStabilization(true);
}

void ego::win32::Win32Window::Accessor::OnWindowSizeUpdate(Win32Window& _window)
{
    _window.updateSizes();
}
