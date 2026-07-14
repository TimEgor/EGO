#include "GuiController.h"

#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareSubsystem.h"

#include "GuiPaintContext.h"

namespace
{
    bool IsGuiMouseInputEvent(ego::gui::GuiInputEventType _type)
    {
        return _type == ego::gui::GuiInputEventType::MouseMove || _type == ego::gui::GuiInputEventType::MouseButtonDown || _type == ego::gui::GuiInputEventType::MouseButtonUp ||
               _type == ego::gui::GuiInputEventType::MouseWheel;
    }
} // namespace

ego::gui::GuiController::~GuiController()
{
    release();
}

bool ego::gui::GuiController::init(const InitData& _initData)
{
    release();

    EGO_CHECK_RETURN_CALL_FALSE(initViewport(_initData.m_viewportDesc), release());

    m_fontAtlas = GuiFontAtlasPointer(new GuiFontAtlas());
    const GraphicDevicePointer graphicDevice = gpu::GetGraphicDevicePointer();
    EGO_CHECK_RETURN_CALL_FALSE(graphicDevice && m_fontAtlas && m_fontAtlas->init(*graphicDevice, _initData.m_fontAtlasDesc), release());

    m_isInitialized = true;
    return true;
}

void ego::gui::GuiController::release()
{
    m_viewport = nullptr;
    m_fontAtlas = nullptr;
    m_mousePosition = GuiPositionZero;
    m_hasMousePosition = false;
    m_isInitialized = false;
}

bool ego::gui::GuiController::initViewport(const GuiViewportDesc& _desc)
{
    EGO_CHECK_RETURN_FALSE(!m_viewport);

    m_viewport = new GuiViewport();
    EGO_CHECK_RETURN_FALSE(m_viewport && m_viewport->init(_desc));

    return true;
}

ego::gui::GuiViewportPointer ego::gui::GuiController::getViewport() const
{
    return m_viewport;
}

void ego::gui::GuiController::beginFrame() {}

void ego::gui::GuiController::endFrame() {}

ego::gui::GuiReply ego::gui::GuiController::processEvent(const GuiInputEvent& _event)
{
    if (!m_isInitialized || !m_viewport)
    {
        return GuiReply::Unhandled();
    }

    GuiInputEvent event = _event;
    if (!prepareInputEvent(event))
    {
        return GuiReply::Unhandled();
    }

    return m_viewport->processEvent(event);
}

bool ego::gui::GuiController::buildDrawData(GuiDrawData& _drawData)
{
    _drawData.clear();

    EGO_CHECK_RETURN_FALSE(m_isInitialized);
    EGO_CHECK_RETURN_FALSE(m_viewport);

    _drawData.m_viewportSize = m_viewport->getSize();

    const GuiWidgetPointer rootWidget = m_viewport->getRootWidget();
    if (!rootWidget)
    {
        return true;
    }

    GuiLayoutContext layoutContext;
    layoutContext.m_scale = m_viewport->getScale();
    layoutContext.m_fontAtlas = m_fontAtlas;

    const GuiRect viewportRect(0.0f, 0.0f, m_viewport->getSize().m_x, m_viewport->getSize().m_y);
    rootWidget->measure(layoutContext, m_viewport->getSize());
    rootWidget->arrange(layoutContext, viewportRect);

    GuiPaintContext paintContext(_drawData, viewportRect, m_fontAtlas);
    rootWidget->paint(paintContext);

    return true;
}

const ego::gui::GuiFontAtlasPointer& ego::gui::GuiController::getFontAtlas() const
{
    return m_fontAtlas;
}

const ego::gui::GuiPosition& ego::gui::GuiController::getMousePosition() const
{
    return m_mousePosition;
}

bool ego::gui::GuiController::hasMousePosition() const
{
    return m_hasMousePosition;
}

bool ego::gui::GuiController::isInitialized() const
{
    return m_isInitialized;
}

bool ego::gui::GuiController::prepareInputEvent(GuiInputEvent& _event)
{
    EGO_CHECK_RETURN_FALSE(m_viewport);

    if (IsGuiMouseInputEvent(_event.m_type))
    {
        const GuiViewportInputAdapterPointer inputAdapter = m_viewport->getInputAdapter();
        if (_event.m_hasPosition && inputAdapter && !inputAdapter->adaptInputEvent(_event, *m_viewport))
        {
            return false;
        }

        return updateMousePosition(_event);
    }

    return true;
}

bool ego::gui::GuiController::updateMousePosition(GuiInputEvent& _event)
{
    if (_event.m_hasPosition)
    {
        m_mousePosition = _event.m_position;
        m_hasMousePosition = true;
        return true;
    }

    if (!m_hasMousePosition)
    {
        return false;
    }

    _event.m_position = m_mousePosition;
    _event.m_hasPosition = true;
    return true;
}
