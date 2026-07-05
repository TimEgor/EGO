#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "GuiInput.h"
#include "GuiViewportInputAdapter.h"
#include "GuiWidget.h"

namespace ego::gui
{
    struct GuiViewportDesc final
    {
        GuiSize m_size = GuiSizeZero;
        float m_scale = 1.0f;
        GuiViewportInputAdapterPointer m_inputAdapter = nullptr;
    };

    class GuiViewport final
    {
    public:
        GuiViewport() = default;

        bool init(const GuiViewportDesc& _desc);

        void setSize(const GuiSize& _size);
        const GuiSize& getSize() const;

        void setScale(float _scale);
        float getScale() const;

        GuiViewportInputAdapterPointer getInputAdapter() const;

        void setRootWidget(const GuiWidgetPointer& _widget);
        GuiWidgetPointer getRootWidget() const;

        GuiReply processEvent(const GuiInputEvent& _event);

        EGO_RTTI_VIRTUAL_BASE(GuiViewport);

    private:
        GuiSize m_size = GuiSizeZero;
        float m_scale = 1.0f;
        GuiViewportInputAdapterPointer m_inputAdapter = nullptr;
        GuiWidgetPointer m_rootWidget = nullptr;
    };

    EGO_POINTER(GuiViewport);
} // namespace ego::gui
