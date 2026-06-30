#include "WindowSystem.h"

#include "EgoApplication/Application.h"

#include "WindowEvents.h"

bool ego::WindowSystem::init()
{
    EventController& eventController = application::GetApplication().getEventController();

    EGO_CHECK_RETURN_FALSE(eventController.registerEvent<WindowSystemQuitRequestedEvent>());
    if (!eventController.registerEvent<WindowDestroyingEvent>())
    {
        eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
        return false;
    }

    if (!eventController.registerEvent<WindowActivationEvent>())
    {
        eventController.unregisterEvent<WindowDestroyingEvent>();
        eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
        return false;
    }

    return true;
}

void ego::WindowSystem::release()
{
    EventController& eventController = application::GetApplication().getEventController();

    eventController.unregisterEvent<WindowActivationEvent>();
    eventController.unregisterEvent<WindowDestroyingEvent>();
    eventController.unregisterEvent<WindowSystemQuitRequestedEvent>();
}

void ego::WindowSystem::EmitQuitRequested()
{
    const WindowSystemQuitRequestedEvent windowSystemEvent;
    application::GetApplication().getEventController().emitEvent(windowSystemEvent);
}

void ego::WindowSystem::EmitWindowDestroying(const WindowPointer& _window)
{
    const WindowDestroyingEvent windowEvent(_window);
    application::GetApplication().getEventController().emitEvent(windowEvent);
}

void ego::WindowSystem::EmitWindowActivate(const WindowPointer& _window, bool _isActive)
{
    const WindowActivationEvent windowEvent(_window, _isActive);
    application::GetApplication().getEventController().emitEvent(windowEvent);
}

void ego::WindowSystem::EmitWindowSizeChange(const WindowPointer& _window, const WindowSize& _prevSize)
{
    const WindowSizeEvent windowEvent(_window, _prevSize);
    application::GetApplication().getEventController().emitInstancedEvent(_window->getSizeEventID(), windowEvent);
}
