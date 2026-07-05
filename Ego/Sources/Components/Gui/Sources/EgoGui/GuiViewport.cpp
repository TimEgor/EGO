#include "GuiViewport.h"

bool ego::gui::GuiViewport::init(const GuiViewportDesc& _desc)
{
    m_size = _desc.m_size;
    m_scale = _desc.m_scale;
    m_inputAdapter = _desc.m_inputAdapter;
    return true;
}

void ego::gui::GuiViewport::setSize(const GuiSize& _size)
{
    m_size = _size;
}

const ego::gui::GuiSize& ego::gui::GuiViewport::getSize() const
{
    return m_size;
}

void ego::gui::GuiViewport::setScale(float _scale)
{
    m_scale = _scale;
}

float ego::gui::GuiViewport::getScale() const
{
    return m_scale;
}

ego::gui::GuiViewportInputAdapterPointer ego::gui::GuiViewport::getInputAdapter() const
{
    return m_inputAdapter;
}

void ego::gui::GuiViewport::setRootWidget(const GuiWidgetPointer& _widget)
{
    m_rootWidget = _widget;
}

ego::gui::GuiWidgetPointer ego::gui::GuiViewport::getRootWidget() const
{
    return m_rootWidget;
}

ego::gui::GuiReply ego::gui::GuiViewport::processEvent(const GuiInputEvent& _event)
{
    if (!m_rootWidget)
    {
        return GuiReply::Unhandled();
    }

    return m_rootWidget->handleEvent(_event);
}
