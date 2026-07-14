#include "WindowSystem.h"

bool ego::WindowSystem::init()
{
    return true;
}

void ego::WindowSystem::release()
{
    m_eventListeners.clear();
}

bool ego::WindowSystem::registerEventListener(const WindowSystemEventListenerPointer& _listener)
{
    if (!_listener)
    {
        return false;
    }

    for (EventListenerCollection::iterator listenerIt = m_eventListeners.begin(); listenerIt != m_eventListeners.end();)
    {
        const WindowSystemEventListenerPointer listener = listenerIt->lock();
        if (!listener)
        {
            listenerIt = m_eventListeners.erase(listenerIt);
            continue;
        }

        if (listener.get() == _listener.get())
        {
            return true;
        }

        ++listenerIt;
    }

    m_eventListeners.emplace_back(_listener);

    return true;
}

void ego::WindowSystem::unregisterEventListener(const WindowSystemEventListenerPointer& _listener)
{
    for (EventListenerCollection::iterator listenerIt = m_eventListeners.begin(); listenerIt != m_eventListeners.end();)
    {
        const WindowSystemEventListenerPointer listener = listenerIt->lock();
        if (!listener || listener.get() == _listener.get())
        {
            listenerIt = m_eventListeners.erase(listenerIt);
            continue;
        }

        ++listenerIt;
    }
}

std::vector<ego::WindowSystemEventListenerPointer> ego::WindowSystem::collectEventListeners() const
{
    std::vector<WindowSystemEventListenerPointer> listeners;
    listeners.reserve(m_eventListeners.size());

    for (const WindowSystemEventListenerWeakPointer& listener : m_eventListeners)
    {
        const WindowSystemEventListenerPointer listenerPointer = listener.lock();
        if (listenerPointer)
        {
            listeners.push_back(listenerPointer);
        }
    }

    return listeners;
}

void ego::WindowSystem::notifyQuitRequested() const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowSystemQuitRequested();
    }
}

void ego::WindowSystem::notifyWindowDestroying(const WindowPointer& _window) const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowDestroying(_window);
    }
}

void ego::WindowSystem::notifyWindowActivate(const WindowPointer& _window, bool _isActive) const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowActivation(_window, _isActive);
    }
}

void ego::WindowSystem::notifyWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize) const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowSizeChanged(_window, _prevSize);
    }
}

void ego::WindowSystem::notifyWindowKeyboardInput(const WindowPointer& _window, const WindowKeyboardInputData& _inputData) const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowKeyboardInput(_window, _inputData);
    }
}

void ego::WindowSystem::notifyWindowTextInput(const WindowPointer& _window, const WindowTextInputData& _inputData) const
{
    const std::vector<WindowSystemEventListenerPointer> listeners = collectEventListeners();
    for (const WindowSystemEventListenerPointer& listener : listeners)
    {
        listener->onWindowTextInput(_window, _inputData);
    }
}
