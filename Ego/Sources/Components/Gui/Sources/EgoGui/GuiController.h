#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoRuntime/Event/EventController.h"

#include "GuiDrawData.h"
#include "GuiFontAtlas.h"
#include "GuiInput.h"
#include "GuiViewport.h"

namespace ego
{
    class GraphicDevice;
    struct InputButtonPressedEvent;
    struct InputButtonReleasedEvent;
    struct InputDeviceChangedEvent;
    struct InputKeyChangedEvent;
} // namespace ego

namespace ego::gui
{
    class GuiController final
    {
    public:
        struct InitData final
        {
            GuiFontAtlasDesc m_fontAtlasDesc;
            GuiViewportDesc m_viewportDesc;
        };

        GuiController() = default;
        ~GuiController();

        bool init(const InitData& _initData);
        void release();

        GuiViewportPointer getViewport() const;

        void beginFrame();
        void endFrame();

        GuiReply processEvent(const GuiInputEvent& _event);
        bool buildDrawData(GuiDrawData& _drawData);

        const GuiFontAtlasPointer& getFontAtlas() const;
        const gpu::Texture2DReference& getFontTexture() const;
        const GuiPosition& getMousePosition() const;
        bool hasMousePosition() const;
        bool isInitialized() const;

        EGO_RTTI_VIRTUAL_BASE(GuiController);

    private:
        bool initFontAtlasRenderResources(GraphicDevice& _graphicDevice);
        bool initViewport(const GuiViewportDesc& _desc);
        bool registerInputEventCallbacks();
        void unregisterInputEventCallbacks();
        bool prepareInputEvent(GuiInputEvent& _event);
        bool updateMousePosition(GuiInputEvent& _event);

        void handleInputDeviceChangedEvent(const InputDeviceChangedEvent& _event);
        void handleInputKeyChangedEvent(const InputKeyChangedEvent& _event);
        void handleInputButtonPressedEvent(const InputButtonPressedEvent& _event);
        void handleInputButtonReleasedEvent(const InputButtonReleasedEvent& _event);

        GuiViewportPointer m_viewport = nullptr;
        GuiFontAtlasPointer m_fontAtlas = nullptr;
        gpu::Texture2DReference m_fontTexture = nullptr;
        GuiPosition m_mousePosition = GuiPositionZero;
        bool m_hasMousePosition = false;
        EventCallbackID m_inputDeviceChangedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputKeyChangedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputButtonPressedCallbackID = InvalidEventCallbackID;
        EventCallbackID m_inputButtonReleasedCallbackID = InvalidEventCallbackID;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
