#include "Input.h"

ego::gui::InputEventSink::~InputEventSink() = default;

ego::gui::InputEvent::~InputEvent() = default;

void ego::gui::PointerMoveEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::PointerExitEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::MouseButtonEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::MouseWheelEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::KeyEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::TextInputEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::PointerCaptureLostEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}

void ego::gui::ViewportDeactivatedEvent::dispatch(InputEventSink& _sink) const
{
    _sink.process(*this);
}
