#include "WindowSystem.h"

bool ego::WindowSystem::init()
{
    return true;
}

void ego::WindowSystem::release()
{
    m_windowSizeChangeHandler = nullptr;
    m_windowActivationHandler = nullptr;
    m_windowDestroyingHandler = nullptr;
    m_quitRequestedHandler = nullptr;
}

void ego::WindowSystem::setQuitRequestedHandler(const WindowSystemQuitRequestedHandler& _handler)
{
    m_quitRequestedHandler = _handler;
}

void ego::WindowSystem::setWindowDestroyingHandler(const WindowSystemWindowDestroyingHandler& _handler)
{
    m_windowDestroyingHandler = _handler;
}

void ego::WindowSystem::setWindowActivationHandler(const WindowSystemWindowActivationHandler& _handler)
{
    m_windowActivationHandler = _handler;
}

void ego::WindowSystem::setWindowSizeChangeHandler(const WindowSystemWindowSizeChangeHandler& _handler)
{
    m_windowSizeChangeHandler = _handler;
}

void ego::WindowSystem::notifyQuitRequested() const
{
    if (m_quitRequestedHandler)
    {
        m_quitRequestedHandler();
    }
}

void ego::WindowSystem::notifyWindowDestroying(const WindowPointer& _window) const
{
    if (m_windowDestroyingHandler)
    {
        m_windowDestroyingHandler(_window);
    }
}

void ego::WindowSystem::notifyWindowActivate(const WindowPointer& _window, bool _isActive) const
{
    if (m_windowActivationHandler)
    {
        m_windowActivationHandler(_window, _isActive);
    }
}

void ego::WindowSystem::notifyWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const
{
    if (m_windowSizeChangeHandler)
    {
        m_windowSizeChangeHandler(_window, _prevSize);
    }
}
