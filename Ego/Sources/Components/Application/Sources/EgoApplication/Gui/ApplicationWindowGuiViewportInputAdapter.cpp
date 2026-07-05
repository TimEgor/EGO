#include "ApplicationWindowGuiViewportInputAdapter.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/GuiViewport.h"

bool ego::application::ApplicationWindowGuiViewportInputAdapter::init(
    const ApplicationWindowPointer& _window,
    const GraphicPresenterPointer& _presenter)
{
    EGO_CHECK_RETURN_FALSE(_window);
    EGO_CHECK_RETURN_FALSE(_presenter);

    m_window = _window;
    m_presenter = _presenter;

    return true;
}

void ego::application::ApplicationWindowGuiViewportInputAdapter::release()
{
    m_presenter = nullptr;
    m_window = nullptr;
}

bool ego::application::ApplicationWindowGuiViewportInputAdapter::adaptInputEvent(
    gui::GuiInputEvent& _event,
    const gui::GuiViewport& _viewport) const
{
    if (!_event.m_hasPosition)
    {
        return true;
    }

    EGO_CHECK_RETURN_FALSE(m_window);

    const WindowSize& clientSize = m_window->getClientAreaSize();
    EGO_CHECK_RETURN_FALSE(clientSize.m_x != 0 && clientSize.m_y != 0);

    WindowPoint screenPoint(
        static_cast<int32_t>(_event.m_position.m_x),
        static_cast<int32_t>(_event.m_position.m_y));
    WindowPoint clientPoint;
    EGO_CHECK_RETURN_FALSE(m_window->screenToClient(screenPoint, clientPoint));

    gui::GuiSize targetSize = _viewport.getSize();
    if (m_presenter)
    {
        const gpu::Texture2DReference targetTexture = m_presenter->getTargetTexture();
        if (targetTexture)
        {
            const gpu::Texture2DSize textureSize = targetTexture->getDesc().m_size;
            if (textureSize.m_x != 0 && textureSize.m_y != 0)
            {
                targetSize = gui::GuiSize(
                    static_cast<float>(textureSize.m_x),
                    static_cast<float>(textureSize.m_y));
            }
        }
    }

    EGO_CHECK_RETURN_FALSE(targetSize.m_x != 0.0f && targetSize.m_y != 0.0f);

    _event.m_position = gui::GuiPosition(
        static_cast<float>(clientPoint.m_x) * targetSize.m_x / static_cast<float>(clientSize.m_x),
        static_cast<float>(clientPoint.m_y) * targetSize.m_y / static_cast<float>(clientSize.m_y));

    return true;
}
