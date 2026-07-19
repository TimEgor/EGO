#include "Input.h"

ego::gui::InputConsumer::~InputConsumer() = default;
ego::gui::InputEvent::~InputEvent() = default;

void ego::gui::PointerMoveEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onPointerMove(*this);
}

void ego::gui::PointerExitEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onPointerExit(*this);
}

void ego::gui::MouseButtonEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onMouseButton(*this);
}

void ego::gui::MouseWheelEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onMouseWheel(*this);
}

void ego::gui::KeyEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onKey(*this);
}

void ego::gui::TextInputEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onTextInput(*this);
}

void ego::gui::ViewportDeactivatedEvent::sendTo(InputConsumer& _consumer) const
{
    _consumer.onViewportDeactivated(*this);
}
