#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "GuiDrawData.h"
#include "GuiFontAtlas.h"
#include "GuiInput.h"
#include "GuiViewport.h"

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
        const GuiPosition& getMousePosition() const;
        bool hasMousePosition() const;
        bool isInitialized() const;

        EGO_RTTI_VIRTUAL_BASE(GuiController);

    private:
        bool initViewport(const GuiViewportDesc& _desc);
        bool prepareInputEvent(GuiInputEvent& _event);
        bool updateMousePosition(GuiInputEvent& _event);

        GuiViewportPointer m_viewport = nullptr;
        GuiFontAtlasPointer m_fontAtlas = nullptr;
        GuiPosition m_mousePosition = GuiPositionZero;
        bool m_hasMousePosition = false;
        bool m_isInitialized = false;
    };

    EGO_POINTER(GuiController);
} // namespace ego::gui
