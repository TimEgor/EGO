#pragma once

#include <vector>

#include "EgoGui/Widgets/GuiWidget.h"

namespace ego::gui
{
    class GuiContainer : public GuiWidget
    {
    public:
        GuiContainer() = default;
        ~GuiContainer() override = default;

        EGO_RTTI_VIRTUAL(GuiContainer, GuiWidget);

    protected:
        using ChildCollection = std::vector<GuiWidgetPointer>;

        bool addChild(const GuiWidgetPointer& _widget);
        void clearChildren();
        void clearHoveredChild(const GuiPosition& _position);
        const ChildCollection& getChildren() const;

        GuiEventResult onEvent(const GuiInputEvent& _event) override;
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override = 0;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override = 0;
        void onPaint(GuiPaintContext& _context) const override;

        virtual bool isChildHitTestVisible(const GuiPosition& _position) const;

    private:
        GuiEventResult processFocusLostEvent(const GuiInputEvent& _event);
        GuiEventResult processPointerEvent(const GuiInputEvent& _event);
        GuiEventResult dispatchToFocusedChild(const GuiInputEvent& _event);

        GuiWidgetPointer findChildAtPosition(const GuiPosition& _position) const;
        void setFocusedChild(const GuiWidgetPointer& _widget);
        void updateHoveredChild(const GuiWidgetPointer& _widget, const GuiPosition& _position);

        ChildCollection m_children;
        GuiWidgetWeakPointer m_focusedChild;
        GuiWidgetWeakPointer m_hoveredChild;
        GuiWidgetWeakPointer m_capturedChild;
        GuiMouseButton m_capturedMouseButton = GuiMouseButton::Left;
    };

    EGO_POINTER(GuiContainer);
} // namespace ego::gui
