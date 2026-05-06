#include "Win32Window.h"

#include "EgoWin32Platform/Win32PlatformUtils.h"

ego::win32::Win32Window::~Win32Window()
{
    release();
}

bool ego::win32::Win32Window::init(const char* _title, const WindowSize& _size)
{
    const Win32Platform& win32Platform = GetWin32FrameworkPlatform();

    EGO_CHECK_INITIALIZATION(initWindowInstancedEvents());

    {
        std::scoped_lock locker(m_mutex);

        m_windowSize = _size;

        m_handle = CreateWindow(
            EGO_WIN32_WND_CLASS_NAME,
            _title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            m_windowSize.m_x,
            m_windowSize.m_y,
            NULL,
            NULL,
            win32Platform.getInstanceHandle(),
            NULL
        );
    }

    EGO_CHECK_INITIALIZATION(m_handle);

    Win32WindowWeakPointer* windowData = new Win32WindowWeakPointer(weakFromThis());
    SetWindowLongPtr(m_handle, 0, reinterpret_cast<LONG_PTR>(windowData));

    ShowCursor(true);

    ShowWindow(m_handle, SW_NORMAL);
    UpdateWindow(m_handle);

    updateSizes();

    return true;
}

void ego::win32::Win32Window::release()
{
    releaseWindowInstancedEvents();

    std::scoped_lock locker(m_mutex);

    if (m_handle)
    {
        DestroyWindow(m_handle);
    }
}

bool ego::win32::Win32Window::isValid() const
{
    std::shared_lock locker(m_mutex);

    return m_handle;
}

void ego::win32::Win32Window::show()
{
    EGO_ASSERT(m_handle);

    std::scoped_lock locker(m_mutex);

    ShowWindow(m_handle, SW_NORMAL);
}

void ego::win32::Win32Window::hide()
{
    EGO_ASSERT(m_handle);

    std::scoped_lock locker(m_mutex);

    ShowWindow(m_handle, SW_HIDE);
}

bool ego::win32::Win32Window::isShown() const
{
    std::shared_lock locker(m_mutex);

    return isValid() && m_isShown;
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

    return isValid() && m_isSizeStable;
}

void ego::win32::Win32Window::updateSizes()
{
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

    Win32WindowWeakPointer* windowData = reinterpret_cast<Win32WindowWeakPointer*>(GetWindowLongPtr(m_handle, 0));
    EGO_SAFE_DESTROY(windowData);

    SetWindowLongPtr(m_handle, 0, 0);

    m_handle = nullptr;
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
