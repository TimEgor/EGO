#include "PlatformWindowSystem.h"

bool ego::PlatformWindowSystem::init()
{
    return true;
}

void ego::PlatformWindowSystem::release()
{
    m_windowSizeChangeHandler = nullptr;
    m_windowActivationHandler = nullptr;
    m_windowDestroyingHandler = nullptr;
    m_quitRequestedHandler = nullptr;
}

void ego::PlatformWindowSystem::setQuitRequestedHandler(const PlatformWindowSystemQuitRequestedHandler& _handler)
{
    m_quitRequestedHandler = _handler;
}

void ego::PlatformWindowSystem::setWindowDestroyingHandler(const PlatformWindowSystemWindowDestroyingHandler& _handler)
{
    m_windowDestroyingHandler = _handler;
}

void ego::PlatformWindowSystem::setWindowActivationHandler(const PlatformWindowSystemWindowActivationHandler& _handler)
{
    m_windowActivationHandler = _handler;
}

void ego::PlatformWindowSystem::setWindowSizeChangeHandler(const PlatformWindowSystemWindowSizeChangeHandler& _handler)
{
    m_windowSizeChangeHandler = _handler;
}

void ego::PlatformWindowSystem::notifyQuitRequested() const
{
    if (m_quitRequestedHandler)
    {
        m_quitRequestedHandler();
    }
}

void ego::PlatformWindowSystem::notifyWindowDestroying(const PlatformWindowPointer& _window) const
{
    if (m_windowDestroyingHandler)
    {
        m_windowDestroyingHandler(_window);
    }
}

void ego::PlatformWindowSystem::notifyWindowActivate(const PlatformWindowPointer& _window, bool _isActive) const
{
    if (m_windowActivationHandler)
    {
        m_windowActivationHandler(_window, _isActive);
    }
}

void ego::PlatformWindowSystem::notifyWindowSizeChange(const PlatformWindowPointer& _window, const PlatformWindowSize& _prevSize) const
{
    if (m_windowSizeChangeHandler)
    {
        m_windowSizeChangeHandler(_window, _prevSize);
    }
}
