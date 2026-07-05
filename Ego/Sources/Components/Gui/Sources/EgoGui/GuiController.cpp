#include "GuiController.h"

#include <cstdint>
#include <vector>

#include "EgoCore/Platform/Input/KeyboardInputDevice.h"
#include "EgoCore/Platform/Input/MouseInputDevice.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoGraphicHardware/GraphicHardwareContext.h"

#include "EgoRuntime/Input/InputEvents.h"
#include "EgoRuntime/RuntimeContext.h"

#include "GuiPaintContext.h"

namespace
{
    ego::EventController* GetCurrentEventController()
    {
        const ego::context::RuntimeContextPointer runtimeContext = ego::context::GetRuntimeContextPointer();
        return runtimeContext ? &runtimeContext->getEventController() : nullptr;
    }

    bool IsMouseDevice(const ego::InputDevicePointer& _device)
    {
        return _device && _device->getType() == ego::MouseInputDevice::GetMetaInfoID();
    }

    bool IsKeyboardDevice(const ego::InputDevicePointer& _device)
    {
        return _device && _device->getType() == ego::KeyboardInputDevice::GetMetaInfoID();
    }

    ego::InputDeviceKey ToInputDeviceKey(ego::MouseInputKey _key)
    {
        return static_cast<ego::InputDeviceKey>(_key);
    }

    ego::gui::GuiPosition GetMousePosition(const ego::InputDevicePointer& _device)
    {
        if (!_device)
        {
            return ego::gui::GuiPositionZero;
        }

        return ego::gui::GuiPosition(
            _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)),
            _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)));
    }

    bool IsMousePositionChanged(const ego::InputDevicePointer& _device)
    {
        if (!_device)
        {
            return false;
        }

        return _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) !=
                   _device->getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisX)) ||
               _device->getValue(ToInputDeviceKey(ego::MouseInputKey::AxisY)) !=
                   _device->getPreviousValue(ToInputDeviceKey(ego::MouseInputKey::AxisY));
    }

    bool TryGetGuiMouseButton(ego::InputDeviceKey _key, ego::gui::GuiMouseButton& _mouseButton)
    {
        if (_key >= ego::MouseInputKeyCount)
        {
            return false;
        }

        switch (static_cast<ego::MouseInputKey>(_key))
        {
            case ego::MouseInputKey::ButtonLeft:
                _mouseButton = ego::gui::GuiMouseButton::Left;
                return true;
            case ego::MouseInputKey::ButtonRight:
                _mouseButton = ego::gui::GuiMouseButton::Right;
                return true;
            case ego::MouseInputKey::ButtonMiddle:
                _mouseButton = ego::gui::GuiMouseButton::Middle;
                return true;
            default:
                return false;
        }
    }

    bool IsMouseWheelKey(ego::InputDeviceKey _key)
    {
        return _key == ToInputDeviceKey(ego::MouseInputKey::Wheel);
    }

    bool IsGuiMouseInputEvent(ego::gui::GuiInputEventType _type)
    {
        return _type == ego::gui::GuiInputEventType::MouseMove ||
               _type == ego::gui::GuiInputEventType::MouseButtonDown ||
               _type == ego::gui::GuiInputEventType::MouseButtonUp ||
               _type == ego::gui::GuiInputEventType::MouseWheel;
    }

    ego::gui::GuiInputEvent CreateMouseEvent(
        ego::gui::GuiInputEventType _type,
        const ego::InputDevicePointer& _device)
    {
        ego::gui::GuiInputEvent event;
        event.m_type = _type;
        event.m_position = GetMousePosition(_device);
        event.m_hasPosition = true;
        return event;
    }

    ego::gui::GuiInputEvent CreateMouseButtonEvent(
        ego::gui::GuiInputEventType _type,
        const ego::InputDevicePointer& _device,
        ego::gui::GuiMouseButton _mouseButton)
    {
        ego::gui::GuiInputEvent event = CreateMouseEvent(_type, _device);
        event.m_mouseButton = _mouseButton;
        return event;
    }

    ego::gui::GuiInputEvent CreateMouseWheelEvent(
        const ego::InputDevicePointer& _device,
        ego::InputDeviceKeyValue _wheelDelta)
    {
        ego::gui::GuiInputEvent event = CreateMouseEvent(ego::gui::GuiInputEventType::MouseWheel, _device);
        event.m_wheelDelta = _wheelDelta;
        return event;
    }

    ego::gui::GuiInputEvent CreateKeyboardEvent(ego::gui::GuiInputEventType _type, ego::InputDeviceKey _key)
    {
        ego::gui::GuiInputEvent event;
        event.m_type = _type;
        event.m_key = _key;
        return event;
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
    EGO_CHECK_RETURN_CALL_FALSE(m_fontAtlas && m_fontAtlas->init(_initData.m_fontAtlasDesc), release());

    const GraphicDevicePointer graphicDevice = gpu::GetGraphicDevicePointer();
    EGO_CHECK_RETURN_CALL_FALSE(graphicDevice && initFontAtlasRenderResources(*graphicDevice), release());

    EGO_CHECK_RETURN_CALL_FALSE(registerInputEventCallbacks(), release());

    m_isInitialized = true;
    return true;
}

void ego::gui::GuiController::release()
{
    unregisterInputEventCallbacks();
    m_viewport = nullptr;
    m_fontTexture = nullptr;
    m_fontAtlas = nullptr;
    m_mousePosition = GuiPositionZero;
    m_hasMousePosition = false;
    m_isInitialized = false;
}

bool ego::gui::GuiController::initFontAtlasRenderResources(GraphicDevice& _graphicDevice)
{
    EGO_CHECK_RETURN_FALSE(m_fontAtlas && m_fontAtlas->isInitialized());

    gpu::Texture2DDesc textureDesc;
    textureDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GraphicResourceUsageTransferDst | gpu::GraphicResourceUsageShaderResource);
    textureDesc.m_size = gpu::Texture2DSize(m_fontAtlas->getWidth(), m_fontAtlas->getHeight());
    textureDesc.m_arrayLayers = 1;
    textureDesc.m_mipLevels = 1;
    textureDesc.m_samples.m_count = 1;
    textureDesc.m_format = gpu::GraphicResourceFormat::R8UNorm;

    const std::vector<uint8_t>& pixels = m_fontAtlas->getPixels();
    const gpu::InitialGraphicResourceData initialData(
        pixels.data(),
        static_cast<uint32_t>(pixels.size()),
        m_fontAtlas->getWidth(),
        m_fontAtlas->getWidth() * m_fontAtlas->getHeight());

    gpu::GpuOperationOptions uploadOptions;
    uploadOptions.m_completionMode = gpu::GpuCompletionMode::WaitForCompletion;

    const gpu::GpuTexture2DTicket textureTicket = _graphicDevice.createAndUploadTexture2D(textureDesc, initialData, uploadOptions);
    EGO_CHECK_RETURN_FALSE(textureTicket.m_resource);
    textureTicket.waitReady();

    m_fontTexture = textureTicket.m_resource;
    return m_fontTexture != nullptr;
}

bool ego::gui::GuiController::initViewport(const GuiViewportDesc& _desc)
{
    EGO_CHECK_RETURN_FALSE(!m_viewport);

    m_viewport = new GuiViewport();
    EGO_CHECK_RETURN_FALSE(m_viewport && m_viewport->init(_desc));

    return true;
}

bool ego::gui::GuiController::registerInputEventCallbacks()
{
    EventController* eventController = GetCurrentEventController();
    EGO_CHECK_RETURN_FALSE(eventController);

    m_inputDeviceChangedCallbackID = eventController->addEventCallback<InputDeviceChangedEvent>(
        [this](const InputDeviceChangedEvent& _event)
        {
            handleInputDeviceChangedEvent(_event);
        });
    EGO_CHECK_RETURN_FALSE(m_inputDeviceChangedCallbackID != InvalidEventCallbackID);

    m_inputKeyChangedCallbackID = eventController->addEventCallback<InputKeyChangedEvent>(
        [this](const InputKeyChangedEvent& _event)
        {
            handleInputKeyChangedEvent(_event);
        });
    EGO_CHECK_RETURN_CALL_FALSE(
        m_inputKeyChangedCallbackID != InvalidEventCallbackID,
        unregisterInputEventCallbacks());

    m_inputButtonPressedCallbackID = eventController->addEventCallback<InputButtonPressedEvent>(
        [this](const InputButtonPressedEvent& _event)
        {
            handleInputButtonPressedEvent(_event);
        });
    EGO_CHECK_RETURN_CALL_FALSE(
        m_inputButtonPressedCallbackID != InvalidEventCallbackID,
        unregisterInputEventCallbacks());

    m_inputButtonReleasedCallbackID = eventController->addEventCallback<InputButtonReleasedEvent>(
        [this](const InputButtonReleasedEvent& _event)
        {
            handleInputButtonReleasedEvent(_event);
        });
    EGO_CHECK_RETURN_CALL_FALSE(
        m_inputButtonReleasedCallbackID != InvalidEventCallbackID,
        unregisterInputEventCallbacks());

    return true;
}

void ego::gui::GuiController::unregisterInputEventCallbacks()
{
    EventController* eventController = GetCurrentEventController();
    if (!eventController)
    {
        m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
        m_inputButtonPressedCallbackID = InvalidEventCallbackID;
        m_inputKeyChangedCallbackID = InvalidEventCallbackID;
        m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
        return;
    }

    if (m_inputButtonReleasedCallbackID != InvalidEventCallbackID)
    {
        eventController->removeEventCallback(m_inputButtonReleasedCallbackID);
        m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputButtonPressedCallbackID != InvalidEventCallbackID)
    {
        eventController->removeEventCallback(m_inputButtonPressedCallbackID);
        m_inputButtonPressedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputKeyChangedCallbackID != InvalidEventCallbackID)
    {
        eventController->removeEventCallback(m_inputKeyChangedCallbackID);
        m_inputKeyChangedCallbackID = InvalidEventCallbackID;
    }

    if (m_inputDeviceChangedCallbackID != InvalidEventCallbackID)
    {
        eventController->removeEventCallback(m_inputDeviceChangedCallbackID);
        m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
    }
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

const ego::gpu::Texture2DReference& ego::gui::GuiController::getFontTexture() const
{
    return m_fontTexture;
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

void ego::gui::GuiController::handleInputDeviceChangedEvent(const InputDeviceChangedEvent& _event)
{
    if (!m_isInitialized || !IsMouseDevice(_event.m_device) || !IsMousePositionChanged(_event.m_device))
    {
        return;
    }

    const GuiInputEvent guiEvent = CreateMouseEvent(GuiInputEventType::MouseMove, _event.m_device);
    processEvent(guiEvent);
}

void ego::gui::GuiController::handleInputKeyChangedEvent(const InputKeyChangedEvent& _event)
{
    if (!m_isInitialized || !IsMouseDevice(_event.m_device) || !IsMouseWheelKey(_event.m_key))
    {
        return;
    }

    const GuiInputEvent guiEvent = CreateMouseWheelEvent(_event.m_device, _event.m_value - _event.m_previousValue);
    processEvent(guiEvent);
}

void ego::gui::GuiController::handleInputButtonPressedEvent(const InputButtonPressedEvent& _event)
{
    if (!m_isInitialized)
    {
        return;
    }

    if (IsMouseDevice(_event.m_device))
    {
        GuiMouseButton mouseButton;
        if (!TryGetGuiMouseButton(_event.m_key, mouseButton))
        {
            return;
        }

        const GuiInputEvent guiEvent =
            CreateMouseButtonEvent(GuiInputEventType::MouseButtonDown, _event.m_device, mouseButton);
        processEvent(guiEvent);
        return;
    }

    if (IsKeyboardDevice(_event.m_device))
    {
        const GuiInputEvent guiEvent = CreateKeyboardEvent(GuiInputEventType::KeyDown, _event.m_key);
        processEvent(guiEvent);
    }
}

void ego::gui::GuiController::handleInputButtonReleasedEvent(const InputButtonReleasedEvent& _event)
{
    if (!m_isInitialized)
    {
        return;
    }

    if (IsMouseDevice(_event.m_device))
    {
        GuiMouseButton mouseButton;
        if (!TryGetGuiMouseButton(_event.m_key, mouseButton))
        {
            return;
        }

        const GuiInputEvent guiEvent =
            CreateMouseButtonEvent(GuiInputEventType::MouseButtonUp, _event.m_device, mouseButton);
        processEvent(guiEvent);
        return;
    }

    if (IsKeyboardDevice(_event.m_device))
    {
        const GuiInputEvent guiEvent = CreateKeyboardEvent(GuiInputEventType::KeyUp, _event.m_key);
        processEvent(guiEvent);
    }
}
