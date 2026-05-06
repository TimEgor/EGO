#include "PlatformEventController.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/Platform/Window/WindowEvents.h"

ego::PlatformEventController::PlatformEventController()
{
    initWindowContextEvent();
}

ego::PlatformEventController::~PlatformEventController()
{
    releaseWindowContextEvent();
}

void ego::PlatformEventController::OnWindowDestroying(WindowPointer _window)
{
    const WindowDestroyingEvent windowEvent(_window);
    engine::GetEngine().getEventController().emitEvent(windowEvent);
}

void ego::PlatformEventController::OnWindowActivate(WindowPointer _window, bool _isActive)
{
    const WindowActivationEvent windowEvent(_window, _isActive);
    engine::GetEngine().getEventController().emitEvent(windowEvent);
}

void ego::PlatformEventController::OnWindowSizeChange(WindowPointer _window, const WindowSize& _prevSize)
{
    const WindowSizeEvent windowEvent(_window, _prevSize);
    engine::GetEngine().getEventController().emitInstancedEvent(_window->getSizeEventID(), windowEvent);
}

void ego::PlatformEventController::initWindowContextEvent()
{
    EventController& eventController = engine::GetEngine().getEventController();

    eventController.registerEvent<WindowDestroyingEvent>();
    eventController.registerEvent<WindowActivationEvent>();
}

void ego::PlatformEventController::releaseWindowContextEvent()
{
    EventController& eventController = engine::GetEngine().getEventController();

    eventController.unregisterEvent<WindowActivationEvent>();
    eventController.unregisterEvent<WindowDestroyingEvent>();
}
